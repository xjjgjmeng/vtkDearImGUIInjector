#include <ImGuiCommon.h>
#include <implot.h>

ImGuiNs::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();

    vtkNew<vtkVolumeProperty> pProperty;
    {
        vtkNew<vtkPiecewiseFunction> pOpacity;
        vtkNew<vtkColorTransferFunction> pColor;
        pProperty->ShadeOn();
        pProperty->SetAmbient(0.30);
        pProperty->SetDiffuse(0.50);
        pProperty->SetSpecular(0.25);
        pProperty->SetSpecularPower(37.5);
        pProperty->SetDisableGradientOpacity(1);

        pProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
        pProperty->SetScalarOpacity(pOpacity);
        pProperty->SetColor(pColor);

        pOpacity->AddPoint(-50.0, 0.0);
        pOpacity->AddPoint(625.49, 0.0);
        pOpacity->AddPoint(1286.34, 0.0);
        pOpacity->AddPoint(1917.15, 0.70);
        pOpacity->AddPoint(2300, 1.0);
        pOpacity->AddPoint(4043.31, 1.0);
        pOpacity->AddPoint(5462.06, 1.0);

        pColor->AddRGBPoint(-50.38,  60/255.,  0,        255/255.);
        pColor->AddRGBPoint(595.45,  91/255.,  76/255.,  141/255.);
        pColor->AddRGBPoint(1196.22, 170/255., 0/255.,   0/255.);
        pColor->AddRGBPoint(1568.38, 208/255., 131/255., 79/255.);
        pColor->AddRGBPoint(2427.80, 235/255., 222/255., 133/255.);
        pColor->AddRGBPoint(2989.06, 255/255., 255/255., 255/255.);
        pColor->AddRGBPoint(4680.69, 1.0,      1.0,      1.0);
    }
    vtkNew<vtkVolume> pVolume;
    vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
#if 1
    pMapper->SetInputData(reader->GetOutput());
#else
    vtkNew<vtkImageFlip> flipImage;
    flipImage->SetInputData(reader->GetOutput());
    flipImage->SetFilteredAxis(2);
    flipImage->Update();
    volumeMapper->SetInputData(flipImage->GetOutput());
#endif
    pMapper->SetBlendModeToComposite();
    pVolume->SetMapper(pMapper);
    pVolume->SetProperty(pProperty);
    ren->AddVolume(pVolume);

    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                logView.Draw();
                ImGui::TreePop();
            }
            //if (ImGui::TreeNodeEx("Volume", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGuiNs::vtkObjSetup("Volume", pVolume);
                //ImGui::TreePop();
            }
            //if (ImGui::TreeNodeEx("Renderer"))
            {
                ImGuiNs::vtkObjSetup("Renderer", ren);
                //ImGui::TreePop();
            }
            //f (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGuiNs::vtkObjSetup("Camera", ren->GetActiveCamera());
                //ImGui::TreePop();
            }
            //ImPlot::ShowDemoWindow();
        };

    // Start rendering app
    ren->SetBackground(0., 0., 0.);
    renWin->Render(); // 非常重要！！

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    ImPlot::CreateContext(); //
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
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();
    ImPlot::DestroyContext(); //
    return 0;
}