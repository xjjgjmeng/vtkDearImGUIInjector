#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    renWin->SetWindowName("OrientationMarkerWidget");
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkAnnotatedCubeActor> cubeActor;
#if 1
    cubeActor->SetXPlusFaceText("L");
    cubeActor->SetXMinusFaceText("R");
    cubeActor->SetYMinusFaceText("I");
    cubeActor->SetYPlusFaceText("S");
    cubeActor->SetZMinusFaceText("P");
    cubeActor->SetZPlusFaceText("A");
    cubeActor->SetXFaceTextRotation(-90);
    cubeActor->SetZFaceTextRotation(90);
#endif
    ren->AddActor(cubeActor);

    vtkNew<vtkAnnotatedCubeActor> axesActor;
#if 1
    axesActor->SetXPlusFaceText("L");
    axesActor->SetXMinusFaceText("R");
    axesActor->SetYMinusFaceText("I");
    axesActor->SetYPlusFaceText("S");
    axesActor->SetZMinusFaceText("P");
    axesActor->SetZPlusFaceText("A");
    axesActor->SetXFaceTextRotation(-90);
    axesActor->SetZFaceTextRotation(90);
#endif
    axesActor->GetTextEdgesProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
    axesActor->GetTextEdgesProperty()->SetLineWidth(2);
    axesActor->GetCubeProperty()->SetColor(colors->GetColor3d("Blue").GetData());
    vtkNew<vtkOrientationMarkerWidget> axes;
    axes->SetViewport(0, 0, 0.1, 0.1);
    axes->SetOrientationMarker(axesActor);
    axes->SetInteractor(iren);
    axes->EnabledOn();
    axes->InteractiveOn();
    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup(u8"立方体", cubeActor, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup(u8"左下角指示器", axesActor);

        if (ImGui::TreeNodeEx("vtkOrientationMarkerWidget", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (float v[3] = { axes->GetOutlineColor()[0],axes->GetOutlineColor()[1],axes->GetOutlineColor()[2] }; ImGui::ColorEdit3("OutlineColor", v))
            {
                axes->SetOutlineColor(v[0], v[1], v[2]);
            }

            ImGui::TreePop();
        }

        {
            auto camera = ren->GetActiveCamera();
            const auto center = cubeActor->GetCenter();
            const auto bounds = cubeActor->GetBounds();
            constexpr auto ratio = 4;

            if (ImGui::Button("Superior"))
            {
                camera->SetViewUp(0, 0, -1);
                camera->SetPosition(center[0], center[1] + (bounds[3] - bounds[2]) * ratio, center[2]);
                camera->SetFocalPoint(center);
                ren->ResetCameraClippingRange();
                cubeActor->SetOrigin(center);

            }
            ImGui::SameLine();
            if (ImGui::Button("Inferior"))
            {
                camera->SetViewUp(0, 0, 1);
                camera->SetPosition(center[0], center[1] - (bounds[3] - bounds[2]) * ratio, center[2]);
                camera->SetFocalPoint(center);
                ren->ResetCameraClippingRange();
                cubeActor->SetOrigin(center);
            }
            ImGui::SameLine();
            if (ImGui::Button("Left"))
            {
                camera->SetViewUp(0, 1, 0);
                camera->SetPosition(center[0] + (bounds[1] - bounds[0]) * ratio, center[1], center[2]);
                camera->SetFocalPoint(center);
                ren->ResetCameraClippingRange();
                cubeActor->SetOrigin(center);
            }
            ImGui::SameLine();
            if (ImGui::Button("Right"))
            {
                camera->SetViewUp(0, 1, 0);
                camera->SetPosition(center[0] - (bounds[1] - bounds[0]) * ratio, center[1], center[2]);
                camera->SetFocalPoint(center);
                ren->ResetCameraClippingRange();
                cubeActor->SetOrigin(center);
            }
            ImGui::SameLine();
            if (ImGui::Button("Anterior"))
            {
                camera->SetViewUp(0, 1, 0);
                camera->SetPosition(center[0], center[1], center[2] + (bounds[5] - bounds[4]) * ratio);
                camera->SetFocalPoint(center);
                ren->ResetCameraClippingRange();
                cubeActor->SetOrigin(center);
            }
            ImGui::SameLine();
            if (ImGui::Button("Posterior"))
            {
                camera->SetViewUp(0, 1, 0);
                camera->SetPosition(center[0], center[1], center[2] - (bounds[5] - bounds[4]) * ratio);
                camera->SetFocalPoint(center);
                ren->ResetCameraClippingRange();
                cubeActor->SetOrigin(center);
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
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    // vtkCameraOrientationWidget
    vtkNew<vtkCameraOrientationWidget> camManipulator;
    camManipulator->SetParentRenderer(ren);
    camManipulator->On();
    auto rep = vtkCameraOrientationRepresentation::SafeDownCast(camManipulator->GetRepresentation());
    rep->AnchorToLowerRight();

    // vtkOrientationMarkerWidget
    vtkNew<vtkAxesActor> iconActor;
    vtkNew<vtkOrientationMarkerWidget> orientationWidget;
    orientationWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
    orientationWidget->SetInteractor(iren);
    orientationWidget->SetOrientationMarker(iconActor);
    orientationWidget->SetViewport(0.8, 0.8, 1., 1.);
    orientationWidget->SetEnabled(1);
    orientationWidget->InteractiveOff();

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

    return 0;
}