﻿#include <ImGuiCommon.h>

//vtkns::LogView logView;

// https://blog.csdn.net/shenziheng1/article/details/54744911

int main(int argc, char* argv[])
{
    SETUP_WINDOW

    vtkNew<vtkImageThreshold> pImageThreshold;
    pImageThreshold->SetInputData(vtkns::getVRData());
    pImageThreshold->ThresholdBetween(-1500, -500);
    pImageThreshold->ReplaceInOn();
    pImageThreshold->SetInValue(255);
    pImageThreshold->ReplaceOutOn();
    pImageThreshold->SetOutValue(0);
    pImageThreshold->Update();

    vtkNew<vtkImageActor> imgActor;
    imgActor->GetMapper()->SetInputConnection(pImageThreshold->GetOutputPort());
    ren->AddViewProp(imgActor);

    auto volume = vtkns::genVR(ren, pImageThreshold->GetOutput(), true);
    volume->AddPosition(150, 0, 0);
    ren->AddViewProp(volume);

    ren->ResetCamera();

    ::imgui_render_callback = [&]
        {
            static bool showOutputImg = false;
            ImGui::Checkbox("ShowOutputImg", &showOutputImg);
            vtkns::vtkObjSetup("ImageThreshold", pImageThreshold, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("volume", volume);
            if (showOutputImg)
            {
                ImGui::Begin("ThresholdOutput");
                vtkns::vtkObjSetup("ImageData", pImageThreshold->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
                ImGui::End();
            }
        };

    // Start rendering app
    ren->SetBackground(0.2, 0.3, 0.4);
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    rwi->EnableRenderOff();
    rwi->Start();

    return 0;
}