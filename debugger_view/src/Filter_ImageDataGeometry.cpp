#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();

    // vtkns::labelWorldZero(ren);

    vtkNew<vtkImageDataGeometryFilter> f;
    f->SetInputData(img);

    vtkNew<vtkWarpScalar> ws;
    ws->SetInputConnection(f->GetOutputPort());
    ws->SetScaleFactor(0.005);

    auto actor = vtkns::genPolyDataActor(ren, ws->GetOutputPort());
    actor->GetMapper()->SetScalarRange(0, 2000);
    //vtkPolyDataMapper::SafeDownCast(actor->GetMapper())->ImmediateModeRenderingOff();
    actor->GetMapper()->SetLookupTable(vtkNew<vtkWindowLevelLookupTable>{});

    ren->ResetCamera();

    ::pWindow = rw;
    ::imgui_render_callback = [&]
        {
            //vtkns::vtkObjSetup("elevation", elevationF, ImGuiTreeNodeFlags_DefaultOpen);
            //vtkns::vtkObjSetup("src", src);
            //vtkns::vtkObjSetup("actor", actor);
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
    return 0;
}