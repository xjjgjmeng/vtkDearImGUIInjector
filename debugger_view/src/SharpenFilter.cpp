#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    SETUP_WINDOW

    vtkNew<ImageSharpenFilter> filter;
    filter->SetInputData(vtkns::getSliceData());

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(filter->GetOutputPort());
    ren->AddActor(actor);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("vtkImageActor", actor);
        if (auto v = filter->GetSharpenCount(); ImGui::SliderInt("Sharpen", &v, 0, 100))
        {
            filter->SetSharpenCount(v);
        }
    };

    // Start rendering app
    ren->SetBackground(0.2, 0.3, 0.4);
    rw->Render();

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(rwi);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
    // 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, rw->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    vtkInteractorStyleSwitch::SafeDownCast(rwi->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    rwi->EnableRenderOff();
    rwi->Start();

    return 0;
}