#include <ImGuiCommon.h>

class MyCameraStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyCameraStyle* New();
    vtkTypeMacro(MyCameraStyle, vtkInteractorStyleTrackballCamera);

    void OnLeftButtonDown() override
    {
        int eventPt[2];
        this->GetInteractor()->GetEventPosition(eventPt);
        ::getLogView()->Add(fmt::format("EventPosition: {}", eventPt));
        if (const auto renderer = this->GetInteractor()->FindPokedRenderer(eventPt[0], eventPt[1]))
        {
            ImGuiNs::printWorldPt(*::getLogView(), renderer, eventPt[0], eventPt[1]);

            this->GetInteractor()->GetPicker()->Pick(eventPt[0], eventPt[1], 0, renderer);
            double pickedPt[3];
            this->GetInteractor()->GetPicker()->GetPickPosition(pickedPt); // 获取pick的世界坐标
            ::getLogView()->Add(fmt::format("pickedPt: {}", pickedPt));

            if (m_b)
            {
                vtkNew<vtkSphereSource> pointSource;
                pointSource->SetCenter(pickedPt);
                pointSource->SetRadius(5.0);
                pointSource->Update();

                vtkNew<vtkPolyDataMapper> mapper;
                mapper->SetInputConnection(pointSource->GetOutputPort());

                m_actor->SetMapper(mapper);
                m_actor->GetProperty()->SetColor(1, 0, 0);
                m_actor->GetProperty()->SetPointSize(5);
                renderer->AddActor(m_actor);
            }
        }

        __super::OnLeftButtonDown();
    }

    void OnLeftButtonUp() override
    {
        __super::OnLeftButtonUp();
    }

    void OnRightButtonDown() override
    {
        m_b = !m_b;
        __super::OnRightButtonDown();
    }

    void OnMouseMove() override
    {
        __super::OnMouseMove();
    }

    vtkNew<vtkActor> m_actor;
    bool m_b = false;;
};
vtkStandardNewMacro(MyCameraStyle);

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    {
        vtkNew<vtkDICOMImageReader> reader;
        reader->SetFileName(ImguiVtkNs::getDicomFile());
        reader->Update();

        vtkNew<vtkImageActor> pActor;
        pActor->SetInputData(reader->GetOutput());
        ren->AddActor(pActor);
    }

    vtkNew<vtkImageSlice> pImgSlice;
    {
        vtkNew<vtkDICOMImageReader> reader;
        reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
        reader->Update();

        vtkNew<vtkImageSliceMapper> pMapper;
        pMapper->SetInputData(reader->GetOutput());
        pMapper->SetOrientationToZ();
        pMapper->SetSliceNumber(200);
        
        pImgSlice->SetMapper(pMapper);
        ren->AddActor(pImgSlice);
    }

    iren->SetInteractorStyle(vtkNew<MyCameraStyle>{});

    {
        auto pPicker = vtkSmartPointer<vtkCellPicker>::New();
        pPicker->SetTolerance(0.0005);
        iren->SetPicker(pPicker);
    }

    //::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            ImGuiNs::vtkObjSetup("CurrentPicker", iren->GetPicker(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::Begin("PickedActor");
            ImGuiNs::vtkObjSetup("ActorInfo", vtkAbstractPropPicker::SafeDownCast(iren->GetPicker())->GetViewProp(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        };

    // Start rendering app
    ren->SetBackground(0.2, 0.3, 0.4);
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
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}