#include <ImGuiCommon.h>

int main()
{
    SETUP_WINDOW

    vtkNew<vtkExtractVOI> voi;
    voi->SetInputData(vtkns::getVRData());
    voi->SetVOI(vtkns::getVRData()->GetExtent());

    vtkNew<vtkImageThreshold> pImageThreshold;
    pImageThreshold->SetInputConnection(voi->GetOutputPort());
    pImageThreshold->ThresholdBetween(-1500, -500);
    pImageThreshold->ReplaceInOn();
    pImageThreshold->SetInValue(255);
    pImageThreshold->ReplaceOutOn();
    pImageThreshold->SetOutValue(0);
    pImageThreshold->Update();

    // vtkMarchingCubes
    vtkNew<vtkMarchingCubes> mc;
    mc->SetInputConnection(pImageThreshold->GetOutputPort());
    mc->ComputeNormalsOn();
    mc->SetValue(0, 255);
    vtkNew<vtkConnectivityFilter> cf;
    cf->SetInputConnection(mc->GetOutputPort());
    cf->SetExtractionModeToLargestRegion();
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(cf->GetOutputPort());
    mapper->ScalarVisibilityOff();
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);
    ren->ResetCamera();

    ::pWindow = rw;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("voi", voi);
            vtkns::vtkObjSetup("MarchingCubes", mc, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("MarchingCubesActor", actor);
        };

    // Start rendering app
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