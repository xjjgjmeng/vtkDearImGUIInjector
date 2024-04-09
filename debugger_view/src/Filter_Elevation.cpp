#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    // vtkns::labelWorldZero(ren);

    vtkNew<vtkSphereSource> src;
    src->SetThetaResolution(51);
    src->SetPhiResolution(17);

    vtkNew<vtkElevationFilter> elevationF;
    elevationF->SetInputConnection(src->GetOutputPort());
    elevationF->SetLowPoint(0, 0, -0.5);
    elevationF->SetHighPoint(0, 0, 0.5);
    elevationF->SetScalarRange(-1, 1);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetScalarRange(-1, 1);
    mapper->SetInputConnection(elevationF->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    vtkNew<vtkScalarBarActor> bar;
    bar->SetLookupTable(mapper->GetLookupTable());
    //bar->SetMaximumHeightInPixels(500);
    ren->AddActor2D(bar);

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("elevation", elevationF, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("src", src);
            vtkns::vtkObjSetup("actor", actor);
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
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();
    return 0;
}