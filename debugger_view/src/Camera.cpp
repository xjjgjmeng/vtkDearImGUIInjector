#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    rw->SetWindowName("OrientationMarkerWidget");

    vtkNew<vtkAnnotatedCubeActor> cubeActor;
    {
        cubeActor->SetXPlusFaceText("L");
        cubeActor->SetXMinusFaceText("R");
        cubeActor->SetYMinusFaceText("I");
        cubeActor->SetYPlusFaceText("S");
        cubeActor->SetZMinusFaceText("P");
        cubeActor->SetZPlusFaceText("A");
        cubeActor->SetXFaceTextRotation(-90);
        cubeActor->SetZFaceTextRotation(90);
        ren->AddActor(cubeActor);
    }

    vtkNew<vtkOrientationMarkerWidget> axes;
    vtkNew<vtkAnnotatedCubeActor> axesActor;
    {
        {
            axesActor->SetXPlusFaceText("L");
            axesActor->SetXMinusFaceText("R");
            axesActor->SetYMinusFaceText("I");
            axesActor->SetYPlusFaceText("S");
            axesActor->SetZMinusFaceText("P");
            axesActor->SetZPlusFaceText("A");
            axesActor->SetXFaceTextRotation(-90);
            axesActor->SetZFaceTextRotation(90);
            axesActor->GetTextEdgesProperty()->SetColor(1, 1, 0);
            axesActor->GetTextEdgesProperty()->SetLineWidth(2);
            axesActor->GetCubeProperty()->SetColor(0,0,1);
            axes->SetOrientationMarker(axesActor);
        }
        axes->SetViewport(0, 0, 0.1, 0.1);
        axes->SetInteractor(rwi);
        axes->EnabledOn();
        axes->InteractiveOn();
    }

    vtkNew<vtkOrientationMarkerWidget> orientationWidget;
    vtkNew<vtkAxesActor> iconActor;
    {
        orientationWidget->SetOutlineColor(0.9300, 0.5700, 0.1300);
        orientationWidget->SetInteractor(rwi);
        orientationWidget->SetOrientationMarker(iconActor);
        orientationWidget->SetViewport(0.8, 0.8, 1., 1.);
        orientationWidget->SetEnabled(1);
        orientationWidget->InteractiveOff();
    }

    ren->ResetCamera();

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup(u8"立方体", cubeActor/*, ImGuiTreeNodeFlags_DefaultOpen*/);
        vtkns::vtkObjSetup(u8"左下角指示器", axesActor);
        
        if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx(u8"右上角指示器", ImGuiTreeNodeFlags_DefaultOpen))
        {
            vtkns::vtkObjSetup("OrientationMarkerWidget", orientationWidget);
            vtkns::vtkObjSetup("AxesActor", iconActor);
        }

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

    AFTER_MY_CODE
}