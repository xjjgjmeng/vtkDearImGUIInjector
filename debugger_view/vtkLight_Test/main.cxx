#include <ImGuiCommon.h>

ImGuiNs::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    auto cylinder = vtkSmartPointer<vtkCylinderSource>::New();
    auto cylindermapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cylindermapper->SetInputConnection(cylinder->GetOutputPort());
    auto cylinderactor = vtkSmartPointer<vtkActor>::New();
    cylinderactor->SetMapper(cylindermapper);
    ren->AddActor(cylinderactor);
    ren->SetBackground(0.1, 0.1, 0.1);

    auto mylight = vtkSmartPointer<vtkLight>::New();
    mylight->SetColor(0, 1, 0);
    mylight->SetPosition(0, 0, 1);
    mylight->SetFocalPoint(ren->GetActiveCamera()->GetFocalPoint());
    ren->AddLight(mylight);

    auto mylight1 = vtkSmartPointer<vtkLight>::New();
    mylight1->SetColor(1, 0, 0);
    mylight1->SetPosition(0, 0, -1);
    mylight1->SetFocalPoint(ren->GetActiveCamera()->GetFocalPoint());
    ren->AddLight(mylight1);
    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            ImGuiNs::vtkObjSetup("Light1", mylight, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("Light2", mylight1, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("Cylinder", cylinderactor, ImGuiTreeNodeFlags_DefaultOpen);
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