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
    auto outlineactor = vtkSmartPointer<vtkActor>::New();
    cylinder->SetResolution(18);
    auto cylindermapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cylindermapper->SetInputConnection(cylinder->GetOutputPort());
    auto cylinderactor = vtkSmartPointer<vtkActor>::New();
    cylinderactor->SetMapper(cylindermapper);

    auto callback = vtkSmartPointer<vtkCallbackCommand>::New();
    callback->SetCallback([](vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
        {
            vtkCylinderSource* cylinderSource = static_cast<vtkCylinderSource*>(caller);
            int resolution = cylinderSource->GetResolution();
            std::cout << __func__ << '\t' << resolution << std::endl;
        });
    cylinder->AddObserver(vtkCommand::ModifiedEvent, callback);

    auto filter = vtkSmartPointer<vtkOutlineFilter>::New();
    filter->SetInputConnection(cylinder->GetOutputPort());
    auto outlinemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outlinemapper->SetInputConnection(filter->GetOutputPort());
    outlineactor->SetMapper(outlinemapper);

    ren->AddActor(cylinderactor);
    ren->AddActor(outlineactor);

    ren->SetBackground(0, 0, 0);

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            static int resolution = cylinder->GetResolution();
            if (ImGui::SliderInt("Resolution", &resolution, 3, 100))
            {
                cylinder->SetResolution(resolution);
            }
            static bool showOutline{ true };
            if (ImGui::Checkbox("ShwOutline", &showOutline))
            {
                if (showOutline)
                {
                    ren->AddActor(outlineactor);
                }
                else
                {
                    ren->RemoveActor(outlineactor);
                }
            }
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