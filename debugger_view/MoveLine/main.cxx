#include <ImGuiCommon.h>

ImGuiNs::LogView logView;

class MyActorStyle : public vtkInteractorStyleTrackballActor
{
public:
    static MyActorStyle* New();
    vtkTypeMacro(MyActorStyle, vtkInteractorStyleTrackballActor);

    void OnLeftButtonDown() override
    {
        __super::OnLeftButtonDown();
        if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
        {
            return;
        }
        this->StopState();
        this->StartPan();
    }
};
vtkStandardNewMacro(MyActorStyle);

class MyActor : public vtkOpenGLActor
{
public:
    static MyActor* New();
    vtkTypeMacro(MyActor, vtkOpenGLActor);

    void Pick() override
    {
        __super::Pick();
    }

    void SetPosition(double pos[3]) override
    {
        ::logView.Add(fmt::format("{}: {} {} {}", __func__, pos[0], pos[1], pos[2]));
        // 只能垂直移动
#if 1
        pos[0] = this->Position[0];
        pos[2] = this->Position[2];
        __super::SetPosition(pos);
#else
        auto pMapper = vtkPolyDataMapper::SafeDownCast(this->GetMapper());
        ::logView.Add(fmt::format("{}", pMapper->GetInput()->GetNumberOfPoints()));
        ::logView.Add(fmt::format("Pt0: {} {} {}", pMapper->GetInput()->GetPoint(0)[0], pMapper->GetInput()->GetPoint(0)[1], pMapper->GetInput()->GetPoint(0)[2]));
        ::logView.Add(fmt::format("Pt1: {} {} {}", pMapper->GetInput()->GetPoint(1)[0], pMapper->GetInput()->GetPoint(1)[1], pMapper->GetInput()->GetPoint(1)[2]));
        pMapper->GetInput()->GetPoint(0)[0] = pMapper->GetInput()->GetPoint(1)[0]= this->Position[0];
        pMapper->GetInput()->GetPoint(0)[1] = pMapper->GetInput()->GetPoint(1)[1]= this->Position[2];
        pMapper->Update();
#endif
    }

    vtkWeakPointer<vtkLineSource> m_pLineSource;
};
vtkStandardNewMacro(MyActor);

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);
    iren->SetInteractorStyle(vtkSmartPointer<MyActorStyle>::New());

    vtkNew<vtkLineSource> lineSource;
    lineSource->SetPoint1(50.0, 50.0, 0.0);  // 起点坐标 
    lineSource->SetPoint2(90.0, 50.0, 0.0);  // 终点坐标
    lineSource->Update();

    vtkNew<vtkPolyDataMapper> lineMapper;
    lineMapper->SetInputConnection(lineSource->GetOutputPort());
    vtkNew<MyActor> lineActor;
    lineActor->SetMapper(lineMapper);
    lineActor->GetProperty()->SetLineStipplePattern(0xAAAA);
    lineActor->GetProperty()->SetLineStippleRepeatFactor(1);
    lineActor->GetProperty()->SetColor(0, 1, 0);
    ren->AddActor(lineActor);

    ::imgui_render_callback = [&]
        {
            ::logView.Draw();
            ImGuiNs::vtkObjSetup("Source", lineSource, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("Mapper", lineMapper, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("Actor", lineActor, ImGuiTreeNodeFlags_DefaultOpen);
        };

    // Start rendering app
    ren->SetBackground(0., 0., 0.);
    renWin->Render(); // 非常重要！！

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(iren);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    ImguiVtkNs::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    ImguiVtkNs::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    vtkNew<vtkCameraOrientationWidget> camManipulator;
    camManipulator->SetParentRenderer(ren);
    camManipulator->On();
    auto rep = vtkCameraOrientationRepresentation::SafeDownCast(camManipulator->GetRepresentation());
    rep->AnchorToLowerRight();

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
// 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, renWin->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}