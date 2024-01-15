#include <ImGuiCommon.h>

class vtkmyPWCallback : public vtkCallbackCommand
{
public:
    vtkmyPWCallback() = default;

    static vtkmyPWCallback* New()
    {
        return new vtkmyPWCallback;
    }
    void Execute(vtkObject* caller, unsigned long, void*) override
    {
        vtkPointWidget* pointWidget = reinterpret_cast<vtkPointWidget*>(caller);
        double position[3];
        pointWidget->GetPosition(position);
        ::getLogView()->Add(fmt::format("Position: {::.2f}", position));
    }
};

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetPhiResolution(15);
    sphereSource->SetThetaResolution(15);
    sphereSource->Update();
    auto inputPolyData = sphereSource->GetOutput();

    vtkNew<vtkPointWidget> pointWidget;
    pointWidget->SetInteractor(iren);
    pointWidget->SetInputData(inputPolyData);
    pointWidget->AllOff();
    pointWidget->PlaceWidget();
    vtkNew<vtkmyPWCallback> myCallback;
    pointWidget->AddObserver(vtkCommand::InteractionEvent, myCallback);
    renWin->Render();
    pointWidget->On();
    ren->ResetCamera();

    ::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        ImGuiNs::vtkObjSetup("PointWidget", pointWidget, ImGuiTreeNodeFlags_DefaultOpen);
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
    { // ??
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballActor();
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}