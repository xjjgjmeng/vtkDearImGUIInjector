#include <ImGuiCommon.h>

int main()
{
    SETUP_WINDOW
    auto img = vtkns::getVRData();

    vtkNew<vtkLookupTable> lut;
    lut->SetTableRange(0, 2000);
    lut->SetHueRange(0, 1); // 灰度图(0,0)
    lut->SetSaturationRange(0, 1);
    lut->SetValueRange(0, 1);
    lut->SetRampToLinear();
    lut->Build();

    vtkNew<vtkImageMapToColors> toColors;
    toColors->SetInputData(img);
    toColors->SetLookupTable(lut);

    vtkNew<vtkImageActor> pActor;
    pActor->SetMapper(vtkNew<vtkImageResliceMapper>{});
    pActor->GetMapper()->SetInputConnection(toColors->GetOutputPort());
    ren->AddActor(pActor);

    auto pVolume = vtkns::genVR(ren, img, false, true);
    vtkns::genImgOutline(ren, img, false);
    vtkns::labelWorldZero(ren);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
        {
            static bool showOutputImg = false;
            if (bool v = pVolume->GetVisibility(); ImGui::Checkbox("VRVisibility", &v)) pVolume->SetVisibility(v);
            ImGui::SameLine();
            ImGui::Checkbox("ShowOutputImg", &showOutputImg);
            vtkns::vtkObjSetup("SlicePlane", pActor->GetMapper()->GetSlicePlane(), ImGuiTreeNodeFlags_DefaultOpen);
            // vtkns::vtkObjSetup("lut", lut, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("ImageMapToColors", toColors, ImGuiTreeNodeFlags_DefaultOpen);

            if (showOutputImg)
            {
                ImGui::Begin("666");
                vtkns::vtkObjSetup("OutputImg", toColors->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen); // 打开此窗口后，调节rangge会变慢
                ImGui::End();
            }
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