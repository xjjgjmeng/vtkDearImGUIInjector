#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkImageGridSource> src;
    src->SetGridSpacing(16, 16, 0);
    src->SetGridOrigin(0, 0, 0);
    src->SetDataExtent(0, 63, 0, 63, 0, 0);
    src->SetLineValue(4095);
    src->SetFillValue(0);
    src->SetDataScalarTypeToShort();

    vtkNew<vtkImageBlend> imgBlend;
    imgBlend->SetOpacity(0, 0.5);
    imgBlend->SetOpacity(1, 0.5);
    imgBlend->AddInputData(vtkns::getSliceData());
    imgBlend->AddInputConnection(src->GetOutputPort());

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(imgBlend->GetOutputPort());
    ren->AddViewProp(actor);

    ren->ResetCamera();

    ::pWindow = rw;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("src", src, ImGuiTreeNodeFlags_DefaultOpen);
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