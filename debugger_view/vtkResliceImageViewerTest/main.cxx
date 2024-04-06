#include <ImGuiCommon.h>

vtkns::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(vtkns::getDicomDir());
    reader->Update();

    vtkNew<vtkResliceImageViewer> viewer;
    auto ren = viewer->GetRenderer();
    auto renWin = viewer->GetRenderWindow();
    vtkNew<vtkRenderWindowInteractor> iren;
    viewer->SetupInteractor(iren);
    viewer->SetInputData(reader->GetOutput());
    viewer->SetSlice(212);
    viewer->SetSliceOrientationToXY();
    viewer->GetRenderer()->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                logView.Draw();
                ImGui::TreePop();
            }
            vtkns::vtkObjSetup("ResliceImageViewer", viewer, ImGuiTreeNodeFlags_DefaultOpen);
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
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}