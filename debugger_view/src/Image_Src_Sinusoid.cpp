#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkImageSinusoidSource> src;
    src->SetWholeExtent(0, 99, 0, 99, 0, 99);
    src->SetAmplitude(63);
    src->SetDirection(1, 0, 0);
    src->SetPeriod(25);
    src->Update();

    vtkns::genImgOutline(ren, src->GetOutput(), false);
    auto pVolume = vtkns::genVR(ren, src->GetOutput(), false);

    ren->ResetCamera();

    ::pWindow = rw;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("volume", pVolume, ImGuiTreeNodeFlags_DefaultOpen);
        };

    // Start rendering app
    ren->SetBackground(0., 0., 0.);
    rw->Render(); // 非常重要！！

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
}