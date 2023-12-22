#include "ImGuiCommon.h"

namespace ImGuiNs
{
    void HelpMarker(const char* desc)
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

    void printWorldPt(ImGuiNs::LogView& logView, vtkRenderer* pRenderer, double disPtX, double disPtY)
    {
        // 0
        {
            double worldPt[4];
            vtkInteractorObserver::ComputeDisplayToWorld(pRenderer, disPtX, disPtY, 0, worldPt);
            logView.Add(fmt::format("worldPt0: {}", worldPt));
        }
        // 1
        {
            vtkNew<vtkCoordinate> coordinate;
            coordinate->SetCoordinateSystemToDisplay();
            coordinate->SetValue(disPtX, disPtY);
            auto worldPt = coordinate->GetComputedWorldValue(pRenderer);
            logView.Add(fmt::format("worldPt1: {}", std::initializer_list{ worldPt[0],worldPt[1], worldPt[2] }));
        }
        // 2
        {
            pRenderer->SetDisplayPoint(disPtX, disPtY, 0);
            pRenderer->DisplayToWorld();
            double worldPt[4];
            pRenderer->GetWorldPoint(worldPt);
            logView.Add(fmt::format("worldPt2: {}", worldPt));
        }
    }

    void vtkObjSetup(vtkSmartPointer<vtkObject> vtkObj)
    {
        if (ImGui::CollapsingHeader("vtkObject", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("MTime: %ld", vtkObj->GetMTime());
            ImGui::Text("Name: %s", vtkObj->GetClassName());
        }

        if (const auto pProp = vtkProp::SafeDownCast(vtkObj); pProp && ImGui::CollapsingHeader("vtkProp", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputScalarN("Bounds", ImGuiDataType_Double, pProp->GetBounds(), 6, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("Matrix0", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 0, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("Matrix1", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 4, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("Matrix2", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 8, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("Matrix3", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 12, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);

            if (bool visibility = pProp->GetVisibility(); ImGui::Checkbox("Visibility ", &visibility))
            {
                pProp->SetVisibility(visibility);
            }
            ImGui::SameLine();
            if (bool pickable = pProp->GetPickable(); ImGui::Checkbox("Pickable", &pickable))
            {
                pProp->SetPickable(pickable);
            }
            ImGui::SameLine();
            if (bool dragable = pProp->GetDragable(); ImGui::Checkbox("Dragable", &dragable))
            {
                pProp->SetDragable(dragable);
            }
        }
        else if (const auto pCamera = vtkCamera::SafeDownCast(vtkObj); pCamera && ImGui::CollapsingHeader("vtkCamera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            constexpr auto myOff = 1;
            {
                double viewup[3];
                pCamera->GetViewUp(viewup);
                if (ImGui::DragScalarN("ViewUp", ImGuiDataType_Double, viewup, 3, 0.01f))
                {
                    pCamera->SetViewUp(viewup);
                }
            }
            {
                double pos[3];
                pCamera->GetPosition(pos);
                if (ImGui::DragScalarN("Position", ImGuiDataType_Double, pos, 3, 0.01f))
                {
                    pCamera->SetPosition(pos);
                }
            }
            {
                double fp[3];
                pCamera->GetFocalPoint(fp);
                if (ImGui::DragScalarN("FocalPoint", ImGuiDataType_Double, fp, 3, 0.01f))
                {
                    pCamera->SetFocalPoint(fp);
                }
            }
            
            if (int myroll = pCamera->GetRoll(); ImGui::DragInt("roll", &myroll, 1.f, -360, 360))
            {
                pCamera->SetRoll(myroll);
            }

            if (ImGui::TreeNode(u8"旋转"))
            {
                {
                    {
                        ImGui::Text("Roll:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Roll_left", ImGuiDir_Left)) { pCamera->Roll(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Roll_right", ImGuiDir_Right)) { pCamera->Roll(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        ImGui::Text("%d", int(pCamera->GetRoll()));
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
                        if (ImGui::ArrowButton("##Dolly_left", ImGuiDir_Left)) { pCamera->Dolly(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Dolly_right", ImGuiDir_Right)) { pCamera->Dolly(myOff); }
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
                        if (ImGui::ArrowButton("##Elevation_left", ImGuiDir_Left)) { pCamera->Elevation(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Elevation_right", ImGuiDir_Right)) { pCamera->Elevation(myOff); }
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
                        if (ImGui::ArrowButton("##Azimuth_left", ImGuiDir_Left)) { pCamera->Azimuth(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Azimuth_right", ImGuiDir_Right)) { pCamera->Azimuth(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        HelpMarker(u8R"(Rotate the camera about the view up vector centered at the focal point.
Note that the view up vector is whatever was set via SetViewUp, and is not necessarily perpendicular to the direction of projection.
The result is a horizontal rotation of the camera.
围绕以focalpoint为中心的viewup旋转相机（只有相机的position在变）
结果是相机的水平旋转)");
                        ImGui::SameLine();
                        {
                            static auto autoAzimuth = false;
                            ImGui::Checkbox("Auto", &autoAzimuth);
                            if (autoAzimuth)
                            {
                                pCamera->Azimuth(-myOff);
                            }
                        }
                    }
                    {
                        ImGui::Text("Yaw:");
                        ImGui::SameLine();
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##Yaw_left", ImGuiDir_Left)) { pCamera->Yaw(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Yaw_right", ImGuiDir_Right)) { pCamera->Yaw(myOff); }
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
                        if (ImGui::ArrowButton("##Pitch_left", ImGuiDir_Left)) { pCamera->Pitch(-myOff); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##Pitch_right", ImGuiDir_Right)) { pCamera->Pitch(myOff); }
                        ImGui::PopButtonRepeat();
                        ImGui::SameLine();
                        HelpMarker(u8R"(Rotate the focal point about the cross product of the view up vector and the direction of projection, using the camera's position as the center of rotation.
The result is a vertical rotation of the camera.)");
                    }
        }
                ImGui::TreePop();
    }
            if (ImGui::Button("OrthogonalizeViewUp"))
            {
                pCamera->OrthogonalizeViewUp();
            }
            ImGui::SameLine();
            if (ImGui::Button("ComputeViewPlaneNormal"))
            {
                pCamera->ComputeViewPlaneNormal();
            }
            {
                static float near_, far_;
                double rangeVal[2];
                pCamera->GetClippingRange(rangeVal);
                near_ = rangeVal[0];
                far_ = rangeVal[1];
                if (ImGui::DragFloatRange2("ClippingRange", &near_, &far_, 0.1f, 0.0f, 100.0f, "Near: %lf", "Far: %lf"))
                {
                    rangeVal[0] = near_;
                    rangeVal[1] = far_;
                    pCamera->SetClippingRange(rangeVal);
                }
            }
            auto orientation = pCamera->GetOrientation();
            ImGui::Text("Orientation:[%lf,%lf,%lf,%lf]", orientation[0], orientation[1], orientation[2], orientation[3]);
            auto dop = pCamera->GetDirectionOfProjection();
            ImGui::Text("DirectionOfProjection:[%lf,%lf,%lf]", dop[0], dop[1], dop[2]);
            {
                ImGui::Text("Distance:");
                ImGui::SameLine();
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##SetDistance_left", ImGuiDir_Left)) { pCamera->SetDistance(pCamera->GetDistance() - myOff); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##SetDistance_right", ImGuiDir_Right)) { pCamera->SetDistance(pCamera->GetDistance() + myOff); }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                ImGui::Text("%lf", pCamera->GetDistance());
            }

            {
                ImGui::Text("ViewAngle:");
                ImGui::SameLine();
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##SetViewAngle_left", ImGuiDir_Left)) { pCamera->SetViewAngle(pCamera->GetViewAngle() - myOff); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##SetViewAngle_right", ImGuiDir_Right)) { pCamera->SetViewAngle(pCamera->GetViewAngle() + myOff); }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                ImGui::Text("%lf", pCamera->GetViewAngle());
            }
            {
                ImGui::Text("Zoom:");
                ImGui::SameLine();
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##Zoom_left", ImGuiDir_Left)) { pCamera->Zoom(0.9); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##Zoom_right", ImGuiDir_Right)) { pCamera->Zoom(1.1); }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                HelpMarker(R"(In perspective mode, decrease the view angle by the specified factor.
In parallel mode, decrease the parallel scale by the specified factor.
A value greater than 1 is a zoom-in, a value less than 1 is a zoom-out.
@note This setting is ignored when UseExplicitProjectionTransformMatrix is true.)");
            }
            if (ImGui::Button(u8"水平翻转"))
            {
                pCamera->Azimuth(180.);
            }ImGui::SameLine();
            if (ImGui::Button(u8"垂直翻转"))
            {
                pCamera->Roll(180.);
                pCamera->Azimuth(180.);
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
                pCamera->SetViewUp(0.0, 1.0, 0.0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"旋转90"))
            {
                pCamera->SetViewUp(-1.0, 0.0, 0.0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"旋转180"))
            {
                pCamera->SetViewUp(0.0, -1.0, 0.0);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"旋转270"))
            {
                pCamera->SetViewUp(1.0, 0.0, 0.0);
            }
        }
        else if (const auto pViewport = vtkViewport::SafeDownCast(vtkObj); pViewport && ImGui::CollapsingHeader("vtkViewport", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (float v[3] = { pViewport->GetBackground()[0],pViewport->GetBackground()[1],pViewport->GetBackground()[2] }; ImGui::ColorEdit3("Background", v))
            {
                pViewport->SetBackground(v[0], v[1], v[2]);
            }
            // ?? 是否可以嵌套
            if (const auto pRenderer = vtkRenderer::SafeDownCast(vtkObj); pRenderer && ImGui::CollapsingHeader("vtkRenderer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("ResetCamera"))
                {
                    pRenderer->ResetCamera();
                }
                ImGui::SameLine();
                if (ImGui::Button("ResetCameraClippingRange"))
                {
                    pRenderer->ResetCameraClippingRange();
                }
                if (bool v = pRenderer->GetUseDepthPeelingForVolumes(); ImGui::Checkbox("UseDepthPeelingForVolumes", &v))
                {
                    pRenderer->SetUseDepthPeelingForVolumes(v);
                }
                if (int v = pRenderer->GetMaximumNumberOfPeels(); ImGui::SliderInt("MaximumNumberOfPeels", &v, 0, 100))
                {
                    pRenderer->SetMaximumNumberOfPeels(v);
                }
                if (float v = pRenderer->GetOcclusionRatio(); ImGui::SliderFloat("OcclusionRatio", &v, 0., 10.))
                {
                    pRenderer->SetOcclusionRatio(v);
                }
            }
        }
        else if (const auto pDataObject = vtkDataObject::SafeDownCast(vtkObj); pDataObject && ImGui::CollapsingHeader("vtkDataObject", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (const auto pDataSet = vtkDataSet::SafeDownCast(vtkObj); pDataSet && ImGui::CollapsingHeader("vtkDataSet", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (const auto pImageData = vtkImageData::SafeDownCast(vtkObj); pImageData && ImGui::CollapsingHeader("vtkImageData", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    {
                        int dims[3];
                        pImageData->GetDimensions(dims);
                        ImGui::Text(fmt::format("Dimensions: {}", dims).c_str());
                    }
                }
            }
        }
        else if (const auto pAbstractPicker = vtkAbstractPicker::SafeDownCast(vtkObj); pAbstractPicker && ImGui::TreeNodeEx("vtkAbstractPicker", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                double v[3];
                pAbstractPicker->GetPickPosition(v);
                ImGui::Text(fmt::format("PickPosition: {::.2f}", v).c_str());
            }
            {
                double v[3];
                pAbstractPicker->GetSelectionPoint(v);
                ImGui::Text(fmt::format("SelectionPoint: {::.2f}", v).c_str());
            }

            if (const auto pAbstractPropPicker = vtkAbstractPropPicker::SafeDownCast(vtkObj); pAbstractPropPicker && ImGui::TreeNodeEx("vtkAbstractPropPicker", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text(fmt::format("ViewProp: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetViewProp())).c_str());
                ImGui::Text(fmt::format("Prop3D: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetProp3D())).c_str());
                ImGui::Text(fmt::format("Actor2D: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetActor2D())).c_str());
                ImGui::Text(fmt::format("Actor: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetActor())).c_str());
                ImGui::Text(fmt::format("Volume: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetVolume())).c_str());
                ImGui::Text(fmt::format("Assembly: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetAssembly())).c_str());
                ImGui::Text(fmt::format("PropAssembly: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetPropAssembly())).c_str());
                ImGui::Text(fmt::format("Path: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetPath())).c_str());

                if (const auto pPicker = vtkPicker::SafeDownCast(vtkObj); pPicker && ImGui::TreeNodeEx("vtkPicker", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (double v = pPicker->GetTolerance(); ImGui::DragScalar("Tolerance", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pPicker->SetTolerance(v);
                    }
                    ImGui::Text(fmt::format("Actors-NumberOfItems: {}", pPicker->GetActors()->GetNumberOfItems()).c_str());
                    ImGui::Text(fmt::format("Prop3Ds-NumberOfItems: {}", pPicker->GetProp3Ds()->GetNumberOfItems()).c_str());

                    if (const auto pPointPicker = vtkPointPicker::SafeDownCast(vtkObj); pPointPicker && ImGui::TreeNodeEx("vtkPointPicker", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text(fmt::format("PointId: {}", pPointPicker->GetPointId()).c_str());
                        ImGui::TreePop();
                    }
                    else if (const auto pCellPicker = vtkCellPicker::SafeDownCast(vtkObj); pCellPicker && ImGui::TreeNodeEx("vtkCellPicker", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text(fmt::format("PointId: {}", pCellPicker->GetPointId()).c_str());
                        ImGui::Text(fmt::format("CellId: {}", pCellPicker->GetCellId()).c_str());
                        ImGui::Text(fmt::format("SubId: {}", pCellPicker->GetSubId()).c_str());
                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
                else if (const auto pPropPicker = vtkPropPicker::SafeDownCast(vtkObj); pPropPicker && ImGui::TreeNodeEx("vtkPropPicker", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
            else if (const auto pWorldPointPicker = vtkWorldPointPicker::SafeDownCast(vtkObj); pWorldPointPicker && ImGui::TreeNodeEx("vtkWorldPointPicker", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        if (const auto pProp3D = vtkProp3D::SafeDownCast(vtkObj); pProp3D && ImGui::CollapsingHeader("vtkProp3D", ImGuiTreeNodeFlags_DefaultOpen))
        {
            double pos[3];
            pProp3D->GetPosition(pos);
            if (ImGui::DragScalarN("Position", ImGuiDataType_Double, pos, 3, 0.1f))
            {
                pProp3D->SetPosition(pos);
            }

            double origin[3];
            pProp3D->GetOrigin(origin);
            if (ImGui::DragScalarN("Origin", ImGuiDataType_Double, origin, 3, 0.1f))
            {
                pProp3D->SetOrigin(origin);
            }

            double scale[3];
            pProp3D->GetScale(scale);
            if (ImGui::DragScalarN("Scale", ImGuiDataType_Double, scale, 3, 0.001f))
            {
                pProp3D->SetScale(scale);
            }

            ImGui::InputScalarN("Center", ImGuiDataType_Double, pProp3D->GetCenter(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("XRange", ImGuiDataType_Double, pProp3D->GetXRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("YRange", ImGuiDataType_Double, pProp3D->GetYRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("ZRange", ImGuiDataType_Double, pProp3D->GetZRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::InputScalarN("Orientation", ImGuiDataType_Double, pProp3D->GetOrientation(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
            ImGui::Text("IsIdentity: %s", pProp3D->GetIsIdentity() ? "true" : "false");

            {
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                const auto n = 5;

                ImGui::Text("RotateX:");
                ImGui::SameLine();
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##leftRotateX", ImGuiDir_Left)) { pProp3D->RotateX(-n); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##rightRotateX", ImGuiDir_Right)) { pProp3D->RotateX(n); }
                ImGui::PopButtonRepeat();

                ImGui::Text("RotateY:");
                ImGui::SameLine();
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##leftRotateY", ImGuiDir_Left)) { pProp3D->RotateY(-n); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##rightRotateY", ImGuiDir_Right)) { pProp3D->RotateY(n); }
                ImGui::PopButtonRepeat();

                ImGui::Text("RotateZ:");
                ImGui::SameLine();
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##leftRotateZ", ImGuiDir_Left)) { pProp3D->RotateZ(-n); }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##rightRotateZ", ImGuiDir_Right)) { pProp3D->RotateZ(n); }
                ImGui::PopButtonRepeat();
            }
        }

        if (const auto pActor = vtkActor::SafeDownCast(vtkObj); pActor && ImGui::CollapsingHeader("vtkActor", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (auto pProperty = pActor->GetProperty(); ImGui::TreeNodeEx("Property", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (bool lighting = pActor->GetProperty()->GetLighting(); ImGui::Checkbox("Lighting", &lighting))
                {
                    pActor->GetProperty()->SetLighting(lighting);
                }
                if (bool f = pActor->GetProperty()->GetEdgeVisibility(); ImGui::Checkbox("EdgeVisibility", &f))
                {
                    pActor->GetProperty()->SetEdgeVisibility(f);
                }
                if (bool f = pActor->GetProperty()->GetVertexVisibility(); ImGui::Checkbox("VertexVisibility", &f))
                {
                    pActor->GetProperty()->SetVertexVisibility(f);
                }
                if (bool f = pActor->GetProperty()->GetRenderPointsAsSpheres(); ImGui::Checkbox("RenderPointsAsSpheres", &f))
                {
                    pActor->GetProperty()->SetRenderPointsAsSpheres(f);
                }
                if (bool f = pActor->GetProperty()->GetRenderLinesAsTubes(); ImGui::Checkbox("RenderLinesAsTubes", &f))
                {
                    pActor->GetProperty()->SetRenderLinesAsTubes(f);
                }
                static float color[3] = { pActor->GetProperty()->GetColor()[0],pActor->GetProperty()->GetColor()[1],pActor->GetProperty()->GetColor()[2] };
                if (ImGui::ColorEdit3("Color", color))
                {
                    pActor->GetProperty()->SetColor(color[0], color[1], color[2]);
                }
                static float edgeColor[3] = { pActor->GetProperty()->GetEdgeColor()[0],pActor->GetProperty()->GetEdgeColor()[1],pActor->GetProperty()->GetEdgeColor()[2] };
                if (ImGui::ColorEdit3("EdgeColor", edgeColor))
                {
                    pActor->GetProperty()->SetEdgeColor(edgeColor[0], edgeColor[1], edgeColor[2]);
                }
                static float vertexColor[3] = { pActor->GetProperty()->GetVertexColor()[0],pActor->GetProperty()->GetVertexColor()[1],pActor->GetProperty()->GetVertexColor()[2] };
                if (ImGui::ColorEdit3("VertexColor", vertexColor))
                {
                    pActor->GetProperty()->SetVertexColor(vertexColor[0], vertexColor[1], vertexColor[2]);
                }
                static float coatColor[3] = { pActor->GetProperty()->GetCoatColor()[0],pActor->GetProperty()->GetCoatColor()[1],pActor->GetProperty()->GetCoatColor()[2] };
                if (ImGui::ColorEdit3("CoatColor", coatColor))
                {
                    pActor->GetProperty()->SetCoatColor(coatColor[0], coatColor[1], coatColor[2]);
                }
                if (float opacity = pActor->GetProperty()->GetOpacity(); ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "opacity = %.3f"))
                {
                    pActor->GetProperty()->SetOpacity(opacity);
                }
                if (float v = pActor->GetProperty()->GetLineWidth(); ImGui::SliderFloat("LineWidth", &v, 0.0f, 30.0f))
                {
                    pActor->GetProperty()->SetLineWidth(v);
                }
                if (float v = pActor->GetProperty()->GetPointSize(); ImGui::SliderFloat("PointSize", &v, 1.0f, 100.0f))
                {
                    pActor->GetProperty()->SetPointSize(v);
                }
                if (float v = pActor->GetProperty()->GetCoatStrength(); ImGui::SliderFloat("CoatStrength", &v, 0.0f, 1.0f))
                {
                    pActor->GetProperty()->SetCoatStrength(v);
                }
                if (float v = pActor->GetProperty()->GetCoatRoughness(); ImGui::SliderFloat("CoatRoughness", &v, 0.0f, 1.0f))
                {
                    pActor->GetProperty()->SetCoatRoughness(v);
                }
                if (float v = pActor->GetProperty()->GetMetallic(); ImGui::SliderFloat("Metallic", &v, 0.0f, 1.0f))
                {
                    pActor->GetProperty()->SetMetallic(v);
                }
                if (int v = pProperty->GetLineStipplePattern(); ImGui::DragInt("LineStipplePattern", &v, 0xFF))
                {
                    pProperty->SetLineStipplePattern(v);
                }
                if (int v = pProperty->GetLineStippleRepeatFactor(); ImGui::SliderInt("LineStippleRepeatFactor", &v, 1, 100))
                {
                    pProperty->SetLineStippleRepeatFactor(v);
                }
                ImGui::TreePop();
            }
        }
        else if (const auto pImageSlice = vtkImageSlice::SafeDownCast(vtkObj); pImageSlice && ImGui::TreeNodeEx("vtkImageSlice"))
        {
            if (const auto pImageActor = vtkImageActor::SafeDownCast(vtkObj); pImageActor && ImGui::TreeNodeEx("vtkImageActor"))
            {
                if (int v[6]; pImageActor->GetDisplayExtent(v), ImGui::DragScalarN("DisplayExtent", ImGuiDataType_S32, v, std::size(v), 0.01f))
                {
                    pImageActor->SetDisplayExtent(v);
                }
                if (double v[6]; pImageActor->GetBounds(v), ImGui::DragScalarN("Bounds", ImGuiDataType_Double, v, std::size(v), 0.01f))
                {
                    //pImageActor->SetBounds(v);
                }
                if (double v[6]; pImageActor->GetDisplayBounds(v), ImGui::DragScalarN("DisplayBounds", ImGuiDataType_Double, v, std::size(v), 0.01f))
                {
                    //pImageActor->SetBounds(v);
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
        else if (const auto pVolume = vtkVolume::SafeDownCast(vtkObj); pVolume && ImGui::TreeNodeEx("vtkVolume", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // mapper
            if (const auto pGPUVolumeRayCastMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(pVolume->GetMapper()); pGPUVolumeRayCastMapper && ImGui::TreeNodeEx("vtkGPUVolumeRayCastMapper", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // BlendMode
                {
                    const char* modeText[] = { "COMPOSITE_BLEND", "MAXIMUM_INTENSITY_BLEND", "MINIMUM_INTENSITY_BLEND", "AVERAGE_INTENSITY_BLEND", "ADDITIVE_BLEND", "ISOSURFACE_BLEND", "SLICE_BLEND" };
                    if (auto v = pGPUVolumeRayCastMapper->GetBlendMode(); ImGui::Combo("BlendMode", &v, modeText, IM_ARRAYSIZE(modeText)))
                    {
                        pGPUVolumeRayCastMapper->SetBlendMode(v);
                    }
                }
                if (float v = pGPUVolumeRayCastMapper->GetImageSampleDistance(); ImGui::SliderFloat("ImageSampleDistance", &v, 0., 10.))
                {
                    pGPUVolumeRayCastMapper->SetImageSampleDistance(v);
                }
                if (float v = pGPUVolumeRayCastMapper->GetSampleDistance(); ImGui::SliderFloat("SampleDistance", &v, 0.01, 3.)) // 调到0.001会崩溃且变卡
                {
                    pGPUVolumeRayCastMapper->SetSampleDistance(v);
                }
                if (bool v = pGPUVolumeRayCastMapper->GetAutoAdjustSampleDistances(); ImGui::Checkbox("AutoAdjustSampleDistances", &v))
                {
                    pGPUVolumeRayCastMapper->SetAutoAdjustSampleDistances(v);
                }
                if (bool v = pGPUVolumeRayCastMapper->GetUseJittering(); ImGui::Checkbox("UseJittering", &v))
                {
                    pGPUVolumeRayCastMapper->SetUseJittering(v);
                }
                ImGui::TreePop();
            }
            // property
            if (const auto pVolumeProperty = pVolume->GetProperty(); pVolumeProperty && ImGui::TreeNodeEx("vtkVolumeProperty", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (bool v = pVolumeProperty->GetShade(); ImGui::Checkbox("ShadeOn", &v))
                {
                    pVolumeProperty->SetShade(v);
                }
                ImGui::SameLine();
                if (bool v = pVolumeProperty->GetDisableGradientOpacity(); ImGui::Checkbox("DisableGradientOpacity", &v))
                {
                    pVolumeProperty->SetDisableGradientOpacity(v);
                }
                if (float v = pVolumeProperty->GetAmbient(); ImGui::SliderFloat("Ambient", &v, 0., 1.))
                {
                    pVolumeProperty->SetAmbient(v);
                }
                if (float v = pVolumeProperty->GetDiffuse(); ImGui::SliderFloat("Diffuse", &v, 0., 1.))
                {
                    pVolumeProperty->SetDiffuse(v);
                }
                if (float v = pVolumeProperty->GetSpecular(); ImGui::SliderFloat("Specular", &v, 0., 1.))
                {
                    pVolumeProperty->SetSpecular(v);
                }
                if (float v = pVolumeProperty->GetSpecularPower(); ImGui::SliderFloat("SpecularPower", &v, 0., 300.))
                {
                    pVolumeProperty->SetSpecularPower(v);
                }
                // InterpolationType
                {
                    const char* modeText[] = { "VTK_NEAREST_INTERPOLATION", "VTK_LINEAR_INTERPOLATION", "VTK_CUBIC_INTERPOLATION" };
                    if (auto v = pVolumeProperty->GetInterpolationType(); ImGui::Combo("InterpolationType", &v, modeText, IM_ARRAYSIZE(modeText)))
                    {
                        pVolumeProperty->SetInterpolationType(v);
                    }
                }
                //ScalarOpacity
                if (ImGui::CollapsingHeader("ScalarOpacity"))
                {
                    const auto pPiecewiseFunction = pVolumeProperty->GetScalarOpacity();
                    if (ImGui::TreeNodeEx("vtkPiecewiseFunction", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        for (auto i = 0; i < pPiecewiseFunction->GetSize(); ++i)
                        {
                            double nodeValue[4];
                            pPiecewiseFunction->GetNodeValue(i, nodeValue);
                            if (float v = nodeValue[1]; ImGui::SliderFloat(fmt::format("{}",nodeValue[0]).c_str(), &v, 0., 1.))
                            {
                                nodeValue[1] = v;
                                pPiecewiseFunction->SetNodeValue(i, nodeValue);
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                if (ImGui::CollapsingHeader("Color"))
                {
                    const auto pColorTransferFunction = pVolumeProperty->GetRGBTransferFunction();
                    if (ImGui::TreeNodeEx("vtkColorTransferFunction", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        for (auto i = 0; i < pColorTransferFunction->GetSize(); ++i)
                        {
                            double nodeValue[6];
                            pColorTransferFunction->GetNodeValue(i, nodeValue);
                            if (float v[3] = { nodeValue[1],nodeValue[2],nodeValue[3] }; ImGui::ColorEdit3(fmt::format("{}", nodeValue[0]).c_str(), v))
                            {
#if 0 // 此代码无效
                                nodeValue[1] = v[0];
                                nodeValue[2] = v[1];
                                nodeValue[3] = v[2];
                                pColorTransferFunction->GetNodeValue(i, nodeValue);
                                pColorTransferFunction->Modified();
#else
                                pColorTransferFunction->AddRGBPoint(nodeValue[0], v[0], v[1], v[2]);
#endif
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
}