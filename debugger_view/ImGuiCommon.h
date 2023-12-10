#pragma once

namespace ImGuiNs
{
    static void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void cameraSetup(vtkSmartPointer<vtkCamera> camera)
    {
        constexpr auto myOff = 1;
        {
            int myroll = camera->GetRoll();
            if (ImGui::DragInt("roll", &myroll, 1.f, -360, 360))
            {
                camera->SetRoll(myroll);
            }
            if (ImGui::Button("Reset"))
            {
                //camera->->ResetCamera();
            }
            ImGui::SameLine();
            if (ImGui::Button("OrthogonalizeViewUp"))
            {
                camera->OrthogonalizeViewUp();
            }
            ImGui::SameLine();
            if (ImGui::Button("ResetCameraClippingRange"))
            {
                //vtkViewer.getRenderer()->ResetCameraClippingRange();
            }
            if (ImGui::Button("ComputeViewPlaneNormal"))
            {
                camera->ComputeViewPlaneNormal();
            }
            double viewup[3];
            camera->GetViewUp(viewup);
            if (ImGui::DragScalarN("ViewUp", ImGuiDataType_Double, viewup, 3, 0.01f))
            {
                camera->SetViewUp(viewup);
            }
            double pos[3];
            camera->GetPosition(pos);
            if (ImGui::DragScalarN("Position", ImGuiDataType_Double, pos, 3, 0.01f))
            {
                camera->SetPosition(pos);
            }
            double fp[3];
            camera->GetFocalPoint(fp);
            if (ImGui::DragScalarN("FocalPoint", ImGuiDataType_Double, fp, 3, 0.01f))
            {
                camera->SetFocalPoint(fp);
            }
            {
                static float near_, far_;
                double rangeVal[2];
                camera->GetClippingRange(rangeVal);
                near_ = rangeVal[0];
                far_ = rangeVal[1];
                if (ImGui::DragFloatRange2("ClippingRange", &near_, &far_, 0.1f, 0.0f, 100.0f, "Near: %lf", "Far: %lf"))
                {
                    rangeVal[0] = near_;
                    rangeVal[1] = far_;
                    camera->SetClippingRange(rangeVal);
                }
            }
            auto orientation = camera->GetOrientation();
            ImGui::Text("Orientation:[%lf,%lf,%lf,%lf]", orientation[0], orientation[1], orientation[2], orientation[3]);
            auto dop = camera->GetDirectionOfProjection();
            ImGui::Text("DirectionOfProjection:[%lf,%lf,%lf]", dop[0], dop[1], dop[2]);
            {
                ImGui::Text("Distance:");
                ImGui::SameLine();
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##SetDistance_left", ImGuiDir_Left)) { camera->SetDistance(camera->GetDistance() - myOff); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##SetDistance_right", ImGuiDir_Right)) { camera->SetDistance(camera->GetDistance() + myOff); }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                ImGui::Text("%lf", camera->GetDistance());
            }
            if (ImGui::TreeNode(u8"旋转"))
            {
                {
                    {
                        ImGui::Text("Roll:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Roll_left", ImGuiDir_Left)) { camera->Roll(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Roll_right", ImGuiDir_Right)) { camera->Roll(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        ImGui::Text("%d", int(camera->GetRoll()));
                        ImGui::SameLine();
                        HelpMarker(u8R"(Rotate the camera about the direction of projection.
This will spin the camera about its axis.
围绕投影方向旋转相机，这将使相机绕其轴旋转)");
                    }
                    {
                        ImGui::Text("Dolly:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Dolly_left", ImGuiDir_Left)) { camera->Dolly(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Dolly_right", ImGuiDir_Right)) { camera->Dolly(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        HelpMarker(R"(Divide the camera's distance from the focal point by the given dolly value.
Use a value greater than one to dolly-in toward the focal point, and use a value less than one to dolly-out away from the focal point.)");
                        //ImGui::SameLine();
                        //ImGui::Text("%d", int(camera->GetDolly()));
                    }
                    {
                        ImGui::Text("Elevation:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Elevation_left", ImGuiDir_Left)) { camera->Elevation(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Elevation_right", ImGuiDir_Right)) { camera->Elevation(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        HelpMarker(R"(Rotate the camera about the cross product of the negative of the direction of projection and the view up vector, using the focal point as the center of rotation.
The result is a vertical rotation of the scene.)");
                    }
                    {
                        ImGui::Text("Azimuth:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Azimuth_left", ImGuiDir_Left)) { camera->Azimuth(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Azimuth_right", ImGuiDir_Right)) { camera->Azimuth(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        HelpMarker(u8R"(Rotate the camera about the view up vector centered at the focal point.
Note that the view up vector is whatever was set via SetViewUp, and is not necessarily perpendicular to the direction of projection.
The result is a horizontal rotation of the camera.
围绕以focalpoint为中心的viewup旋转相机（只有相机的position在变）
结果是相机的水平旋转)");
                    }
                    {
                        ImGui::Text("Yaw:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Yaw_left", ImGuiDir_Left)) { camera->Yaw(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Yaw_right", ImGuiDir_Right)) { camera->Yaw(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        HelpMarker(u8R"(Rotate the focal point about the view up vector, using the camera's position as the center of rotation.
Note that the view up vector is whatever was set via SetViewUp, and is not necessarily perpendicular to the direction of projection.
The result is a horizontal rotation of the scene.
使用相机position作为旋转中心，围绕viewup旋转focalpoint（只有focalpoint在变）
结果是场景的水平旋转)");
                    }
                    {
                        ImGui::Text("Pitch:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Pitch_left", ImGuiDir_Left)) { camera->Pitch(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Pitch_right", ImGuiDir_Right)) { camera->Pitch(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        HelpMarker(u8R"(Rotate the focal point about the cross product of the view up vector and the direction of projection, using the camera's position as the center of rotation.
The result is a vertical rotation of the camera.)");
                    }
                }
                ImGui::TreePop();
            }

            {
                ImGui::Text("ViewAngle:");
                ImGui::SameLine();
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##SetViewAngle_left", ImGuiDir_Left)) { camera->SetViewAngle(camera->GetViewAngle() - myOff); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##SetViewAngle_right", ImGuiDir_Right)) { camera->SetViewAngle(camera->GetViewAngle() + myOff); }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                ImGui::Text("%lf", camera->GetViewAngle());
            }
            {
                ImGui::Text("Zoom:");
                ImGui::SameLine();
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##Zoom_left", ImGuiDir_Left)) { camera->Zoom(0.9); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##Zoom_right", ImGuiDir_Right)) { camera->Zoom(1.1); }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                HelpMarker(R"(In perspective mode, decrease the view angle by the specified factor.
In parallel mode, decrease the parallel scale by the specified factor.
A value greater than 1 is a zoom-in, a value less than 1 is a zoom-out.
@note This setting is ignored when UseExplicitProjectionTransformMatrix is true.)");
            }
            if (ImGui::Button(u8"水平翻转"))
            {
                camera->Azimuth(180.);
            }ImGui::SameLine();
            if (ImGui::Button(u8"垂直翻转"))
            {
                camera->Roll(180.);
                camera->Azimuth(180.);
            }ImGui::SameLine();
            if (ImGui::Button("Mirror"))
            {
                //double imageActorBound[6]{ 0 };
                //panData->getImageActor()->GetBounds(imageActorBound);
                //double* pCenter = panData->getImageActor()->GetCenter();
                //panData->getRenderer()->GetActiveCamera()->SetPosition(pCenter[0], pCenter[1], -1* panData->getRenderer()->GetActiveCamera()->GetDistance());
            }
            if (ImGui::Button(u8"旋转0"))
            {
                camera->SetViewUp(0.0, 1.0, 0.0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"旋转90"))
            {
                camera->SetViewUp(-1.0, 0.0, 0.0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"旋转180"))
            {
                camera->SetViewUp(0.0, -1.0, 0.0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"旋转270"))
            {
                camera->SetViewUp(1.0, 0.0, 0.0);
            }
#if 0
            {
                const auto center = cube->GetCenter();
                const auto bounds = cube->GetBounds();
                constexpr auto ratio = 4;

                if (ImGui::Button("Superior"))
                {
                    camera->SetViewUp(0, 0, -1);
                    camera->SetPosition(center[0], center[1] + (bounds[3] - bounds[2]) * ratio, center[2]);
                    camera->SetFocalPoint(center);
                    vtkViewer.getRenderer()->ResetCameraClippingRange();
                    cube->SetOrigin(center);

                }
                ImGui::SameLine();
                if (ImGui::Button("Inferior"))
                {
                    camera->SetViewUp(0, 0, 1);
                    camera->SetPosition(center[0], center[1] - (bounds[3] - bounds[2]) * ratio, center[2]);
                    camera->SetFocalPoint(center);
                    vtkViewer.getRenderer()->ResetCameraClippingRange();
                    cube->SetOrigin(center);
                }
                ImGui::SameLine();
                if (ImGui::Button("Left"))
                {
                    camera->SetViewUp(0, 1, 0);
                    camera->SetPosition(center[0] + (bounds[1] - bounds[0]) * ratio, center[1], center[2]);
                    camera->SetFocalPoint(center);
                    vtkViewer.getRenderer()->ResetCameraClippingRange();
                    cube->SetOrigin(center);
                }
                ImGui::SameLine();
                if (ImGui::Button("Right"))
                {
                    camera->SetViewUp(0, 1, 0);
                    camera->SetPosition(center[0] - (bounds[1] - bounds[0]) * ratio, center[1], center[2]);
                    camera->SetFocalPoint(center);
                    vtkViewer.getRenderer()->ResetCameraClippingRange();
                    cube->SetOrigin(center);
                }
                ImGui::SameLine();
                if (ImGui::Button("Anterior"))
                {
                    camera->SetViewUp(0, 1, 0);
                    camera->SetPosition(center[0], center[1], center[2] + (bounds[5] - bounds[4]) * ratio);
                    camera->SetFocalPoint(center);
                    vtkViewer.getRenderer()->ResetCameraClippingRange();
                    cube->SetOrigin(center);
                }
                ImGui::SameLine();
                if (ImGui::Button("Posterior"))
                {
                    camera->SetViewUp(0, 1, 0);
                    camera->SetPosition(center[0], center[1], center[2] - (bounds[5] - bounds[4]) * ratio);
                    camera->SetFocalPoint(center);
                    vtkViewer.getRenderer()->ResetCameraClippingRange();
                    cube->SetOrigin(center);
                }
            }
#endif
        }
    }
}