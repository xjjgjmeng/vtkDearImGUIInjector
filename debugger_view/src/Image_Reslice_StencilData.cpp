#include <ImGuiCommon.h>

int main()
{
    SETUP_WINDOW
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    auto reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputData(img);
    reslice->SetOutputDimensionality(2);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false);

    {
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        reslice->SetResliceAxesDirectionCosines(x, y, z);
        reslice->SetResliceAxesOrigin(img->GetCenter());
    }
    reslice->SetInterpolationModeToLinear();
    reslice->SetStencilData(vtkNew<vtkImageStencilData>{});


    // 用于显示reslice生成的影像
    vtkNew<vtkImageActor> resliceImgActor;
    resliceImgActor->GetProperty()->SetColorWindow(6000);
    resliceImgActor->GetProperty()->SetColorLevel(2000);
    resliceImgActor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
    ren->AddActor(resliceImgActor);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("Relice", reslice, ImGuiTreeNodeFlags_DefaultOpen);
    };

    // Start rendering app
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
}