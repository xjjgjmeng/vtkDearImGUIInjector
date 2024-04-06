#include <ImGuiCommon.h>

namespace {
    // This does the actual work.
    // Callback for the interaction
    class vtkLineCallback : public vtkCommand
    {
    public:
        static vtkLineCallback* New()
        {
            return new vtkLineCallback;
        }

        virtual void Execute(vtkObject* caller, unsigned long, void*)
        {

            vtkLineWidget2* lineWidget = reinterpret_cast<vtkLineWidget2*>(caller);

            // Get the actual box coordinates of the line
            vtkNew<vtkPolyData> polydata;
            static_cast<vtkLineRepresentation*>(lineWidget->GetRepresentation())
                ->GetPolyData(polydata);

            // Display one of the points, just so we know it's working
            double p[3];
            polydata->GetPoint(0, p);
            std::cout << "P: " << p[0] << " " << p[1] << " " << p[2] << std::endl;
        }
        vtkLineCallback()
        {
        }
    };
} // namespace

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
        ::pLogView->Add(fmt::format("{}: {} {} {}", __func__, pos[0], pos[1], pos[2]));
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

    vtkNew<vtkLineWidget2> lineWidget;
    {
        lineWidget->SetInteractor(iren);
        lineWidget->CreateDefaultRepresentation();
        double pt1[3]{ 50.0, 60.0, 0.0 };
        double pt2[3]{ 90.0, 60.0, 0.0 };
        lineWidget->GetLineRepresentation()->SetPoint1WorldPosition(pt1);
        lineWidget->GetLineRepresentation()->SetPoint2WorldPosition(pt2);
        vtkNew<vtkLineCallback> lineCallback;
        lineWidget->AddObserver(vtkCommand::InteractionEvent, lineCallback);
        renWin->Render();
        lineWidget->On();
    }

    ::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("Source", lineSource, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("Mapper", lineMapper, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("Actor", lineActor, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("LineWidget2", lineWidget, ImGuiTreeNodeFlags_DefaultOpen);
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
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
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