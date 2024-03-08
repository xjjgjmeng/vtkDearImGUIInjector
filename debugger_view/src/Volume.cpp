#include <ImGuiCommon.h>
#include <implot.h>

namespace {
    class vtkBoxCallback : public vtkCommand
    {
    public:
        static vtkBoxCallback* New()
        {
            return new vtkBoxCallback;
        }

        void Execute(vtkObject* caller, unsigned long, void*) override
        {
            auto boxWidget = vtkBoxWidget2::SafeDownCast(caller);
            vtkNew<vtkTransform> t;
            vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation())->GetTransform(t);
            this->m_actor->SetUserTransform(t);
        }

        vtkSmartPointer<vtkProp3D> m_actor;
    };
} // namespace

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();

    vtkNew<vtkVolume> pVolume;
    vtkNew<vtkGPUVolumeRayCastMapper> pMapper;

#define vtkImageFlip_flag 0
#define vtkExtractVOI_flag 0

#if 1 == vtkImageFlip_flag
    vtkNew<vtkImageFlip> pImageFlip;
    pImageFlip->SetInputData(reader->GetOutput());
    pImageFlip->SetFilteredAxis(2);
    pImageFlip->Update();
    pMapper->SetInputData(pImageFlip->GetOutput());
#elif 1 == vtkExtractVOI_flag
    vtkNew<vtkExtractVOI> pExtractVOI;
    pExtractVOI->SetInputData(reader->GetOutput());
    pExtractVOI->SetVOI(200, 600, 200, 600, 200, 600);
    pExtractVOI->Update();
    pMapper->SetInputData(pExtractVOI->GetOutput());
#else
    pMapper->SetInputData(reader->GetOutput());
#endif
    pMapper->SetBlendModeToComposite();
    pVolume->SetMapper(pMapper);
    ::setupDefaultVolumeProperty(pVolume);
    ren->AddVolume(pVolume);

    vtkNew<vtkVolumeOutlineSource> pVOS;
    vtkNew<vtkActor> vosActor;
    {
        pVOS->SetVolumeMapper(pMapper);
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(pVOS->GetOutputPort());
        vosActor->SetMapper(mapper);
        ren->AddActor(vosActor);
    }

    vtkNew<vtkBoxWidget2> boxWidget;
    {
        boxWidget->SetInteractor(iren);
        boxWidget->GetRepresentation()->SetPlaceFactor(1); // Default is 0.5
        boxWidget->GetRepresentation()->PlaceWidget(pVolume->GetBounds());
        boxWidget->RotationEnabledOff();

        vtkNew<vtkBoxCallback> boxCallback;
        boxCallback->m_actor = pVolume;
        boxWidget->AddObserver(vtkCommand::InteractionEvent, boxCallback);

        boxWidget->On();
    }

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            ImGuiNs::vtkObjSetup("Volume", pVolume, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("OutlineSrc", pVOS, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("Box", boxWidget, ImGuiTreeNodeFlags_DefaultOpen);
#if 1 == vtkImageFlip_flag
            ImGuiNs::vtkObjSetup("ImageFlip", pImageFlip, ImGuiTreeNodeFlags_DefaultOpen);
#elif 1 == vtkExtractVOI_flag
            ImGuiNs::vtkObjSetup("ExtractVOI", pExtractVOI, ImGuiTreeNodeFlags_DefaultOpen);
#endif
            ImGuiNs::vtkObjSetup("vosActor", vosActor);
            ImGuiNs::vtkObjSetup("Mapper", pMapper);
            //ImPlot::ShowDemoWindow();
        };

    // Start rendering app
    ren->SetBackground(0., 0., 0.);
    renWin->Render(); // 非常重要！！

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    ImPlot::CreateContext(); //
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
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();
    ImPlot::DestroyContext(); //
    return 0;
}