#include <ImGuiCommon.h>

ImGuiNs::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkDistanceWidget> distanceWidget;
    {
        distanceWidget->SetInteractor(iren);
        distanceWidget->CreateDefaultRepresentation();
        vtkDistanceRepresentation::SafeDownCast(distanceWidget->GetRepresentation())->SetLabelFormat("%-#6.3g mm");
        renWin->Render();
        distanceWidget->On();
    }

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        if (ImGui::TreeNode("Log"))
        {
            logView.Draw();
            ImGui::TreePop();
        }
        ImGuiNs::vtkObjSetup("DistanceWidget", distanceWidget, ImGuiTreeNodeFlags_DefaultOpen);
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
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballActor();
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}