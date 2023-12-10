#include "../IncludeAllInOne.h"
#include "../ImGuiCommon.h"

#ifdef ADOBE_IMGUI_SPECTRUM
#include "imgui_spectrum.h"
#endif
#include "vtkOpenGLRenderWindow.h" // needed to check if opengl is supported.

// Listens to vtkDearImGuiInjector::ImGuiSetupEvent
static void SetupUI(vtkDearImGuiInjector*);
// Listens to vtkDearImGuiInjector::ImGuiDrawEvent
static void DrawUI(vtkDearImGuiInjector*);
static void HelpMarker(const char* desc);

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

//vtkSmartPointer<vtkImageReslice> reslice;
//vtkSmartPointer<vtkImageMapToColors> colorMap;
//vtkSmartPointer<vtkRenderWindowInteractor> iren;
//vtkSmartPointer<vtkRenderWindow> renderWindow;
//vtkSmartPointer<vtkRenderer> renderer;
//vtkSmartPointer<vtkDICOMImageReader> reader;
//vtkSmartPointer<vtkImageActor> actor;
//double spacing[3];

vtkSmartPointer<vtkActor> coneactor;
vtkSmartPointer<vtkRenderer> renderer;

int main(int argc, char* argv[])
{
  // Create a renderer, render window, and interactor
  //vtkNew<vtkRenderer> renderer;
  renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> iren;
  renderWindow->SetMultiSamples(8);
  renderWindow->AddRenderer(renderer);
  iren->SetRenderWindow(renderWindow);

  // Create pipeline
  vtkNew<vtkConeSource> coneSource;
  coneSource->Update();

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(coneSource->GetOutputPort());

  coneactor = vtkSmartPointer<vtkActor>::New();
  coneactor->SetMapper(mapper);

  // Add the actors to the scene
  renderer->AddActor(coneactor);

  // Start rendering app
  renderer->SetBackground(0.2, 0.3, 0.4);
  renderWindow->Render();

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(iren);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    vtkNew<vtkCameraOrientationWidget> camManipulator;
    camManipulator->SetParentRenderer(renderer);
    camManipulator->On();
    auto rep = vtkCameraOrientationRepresentation::SafeDownCast(camManipulator->GetRepresentation());
    rep->AnchorToLowerRight();

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
// 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, renderWindow->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}

static void SetupUI(vtkDearImGuiInjector* overlay)
{
    vtkNew<vtkCallbackCommand> uiSetup;
    auto uiSetupFunction =
        [](vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* callData)
        {
            vtkDearImGuiInjector* overlay_ = reinterpret_cast<vtkDearImGuiInjector*>(caller);
            if (!callData)
            {
                return;
            }
            bool imguiInitStatus = *(reinterpret_cast<bool*>(callData));
            if (imguiInitStatus)
            {
                auto io = ImGui::GetIO();
                io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf", 15.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
            }
        };
    uiSetup->SetCallback(uiSetupFunction);
    overlay->AddObserver(vtkDearImGuiInjector::ImGuiSetupEvent, uiSetup);
}

static void DrawUI(vtkDearImGuiInjector* overlay)
{
    vtkNew<vtkCallbackCommand> uiDraw;
    auto uiDrawFunction = [](vtkObject* caller, long unsigned int vtkNotUsed(eventId),
        void* clientData, void* vtkNotUsed(callData))
        {
            vtkDearImGuiInjector* overlay_ = reinterpret_cast<vtkDearImGuiInjector*>(caller);

            ImGui::SetNextWindowPos(ImVec2(0, 25), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_Once);
            ImGui::Begin("VTK");
            {
                if (ImGui::BeginTabBar("MyTabBar"))
                {
                    if (ImGui::BeginTabItem("Prop"))
                    {
                        ImGui::Text("MTime: %ld", ::coneactor->GetMTime());
                        ImGui::Text("Name: %s", ::coneactor->GetClassName());
                        if (ImGui::CollapsingHeader("vtkProp", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::InputScalarN("Bounds", ImGuiDataType_Double, ::coneactor->GetBounds(), 6, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("Matrix0", ImGuiDataType_Double, ::coneactor->GetMatrix()->GetData() + 0, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("Matrix1", ImGuiDataType_Double, ::coneactor->GetMatrix()->GetData() + 4, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("Matrix2", ImGuiDataType_Double, ::coneactor->GetMatrix()->GetData() + 8, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("Matrix3", ImGuiDataType_Double, ::coneactor->GetMatrix()->GetData() + 12, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);

                            if (bool visibility = ::coneactor->GetVisibility(); ImGui::Checkbox("Visibility ", &visibility))
                            {
                                ::coneactor->SetVisibility(visibility);
                            }
                            ImGui::SameLine();
                            if (bool pickable = ::coneactor->GetPickable(); ImGui::Checkbox("Pickable", &pickable))
                            {
                                ::coneactor->SetPickable(pickable);
                            }
                            ImGui::SameLine();
                            if (bool dragable = ::coneactor->GetDragable(); ImGui::Checkbox("Dragable", &dragable))
                            {
                                ::coneactor->SetDragable(dragable);
                            }
                        }

                        if (ImGui::CollapsingHeader("vtkProp3D", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            double pos[3];
                            ::coneactor->GetPosition(pos);
                            if (ImGui::DragScalarN("Position", ImGuiDataType_Double, pos, 3, 0.001f))
                            {
                                ::coneactor->SetPosition(pos);
                            }

                            double origin[3];
                            ::coneactor->GetOrigin(origin);
                            if (ImGui::DragScalarN("Origin", ImGuiDataType_Double, origin, 3, 0.001f))
                            {
                                ::coneactor->SetOrigin(origin);
                            }

                            double scale[3];
                            ::coneactor->GetScale(scale);
                            if (ImGui::DragScalarN("Scale", ImGuiDataType_Double, scale, 3, 0.001f))
                            {
                                ::coneactor->SetScale(scale);
                            }

                            ImGui::InputScalarN("Center", ImGuiDataType_Double, ::coneactor->GetCenter(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("XRange", ImGuiDataType_Double, ::coneactor->GetXRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("YRange", ImGuiDataType_Double, ::coneactor->GetYRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("ZRange", ImGuiDataType_Double, ::coneactor->GetZRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputScalarN("Orientation", ImGuiDataType_Double, ::coneactor->GetOrientation(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                            ImGui::Text("IsIdentity: %s", ::coneactor->GetIsIdentity() ? "true" : "false");

                            {
                                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                                const auto n = 5;

                                ImGui::Text("RotateX:");
                                ImGui::SameLine();
                                ImGui::PushButtonRepeat(true);
                                if (ImGui::ArrowButton("##leftRotateX", ImGuiDir_Left)) { ::coneactor->RotateX(-n); }
                                ImGui::SameLine(0.0f, spacing);
                                if (ImGui::ArrowButton("##rightRotateX", ImGuiDir_Right)) { ::coneactor->RotateX(n); }
                                ImGui::PopButtonRepeat();

                                ImGui::Text("RotateY:");
                                ImGui::SameLine();
                                ImGui::PushButtonRepeat(true);
                                if (ImGui::ArrowButton("##leftRotateY", ImGuiDir_Left)) { ::coneactor->RotateY(-n); }
                                ImGui::SameLine(0.0f, spacing);
                                if (ImGui::ArrowButton("##rightRotateY", ImGuiDir_Right)) { ::coneactor->RotateY(n); }
                                ImGui::PopButtonRepeat();

                                ImGui::Text("RotateZ:");
                                ImGui::SameLine();
                                ImGui::PushButtonRepeat(true);
                                if (ImGui::ArrowButton("##leftRotateZ", ImGuiDir_Left)) { ::coneactor->RotateZ(-n); }
                                ImGui::SameLine(0.0f, spacing);
                                if (ImGui::ArrowButton("##rightRotateZ", ImGuiDir_Right)) { ::coneactor->RotateZ(n); }
                                ImGui::PopButtonRepeat();
                            }
                        }

                        if (ImGui::CollapsingHeader("vtkActor", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (ImGui::TreeNodeEx("Property", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                if (bool lighting = ::coneactor->GetProperty()->GetLighting(); ImGui::Checkbox("Lighting", &lighting))
                                {
                                    ::coneactor->GetProperty()->SetLighting(lighting);
                                }
                                if (bool f = ::coneactor->GetProperty()->GetEdgeVisibility(); ImGui::Checkbox("EdgeVisibility", &f))
                                {
                                    ::coneactor->GetProperty()->SetEdgeVisibility(f);
                                }
                                if (bool f = ::coneactor->GetProperty()->GetVertexVisibility(); ImGui::Checkbox("VertexVisibility", &f))
                                {
                                    ::coneactor->GetProperty()->SetVertexVisibility(f);
                                }
                                if (bool f = ::coneactor->GetProperty()->GetRenderPointsAsSpheres(); ImGui::Checkbox("RenderPointsAsSpheres", &f))
                                {
                                    ::coneactor->GetProperty()->SetRenderPointsAsSpheres(f);
                                }
                                if (bool f = ::coneactor->GetProperty()->GetRenderLinesAsTubes(); ImGui::Checkbox("RenderLinesAsTubes", &f))
                                {
                                    ::coneactor->GetProperty()->SetRenderLinesAsTubes(f);
                                }
                                static float color[3] = { ::coneactor->GetProperty()->GetColor()[0],::coneactor->GetProperty()->GetColor()[1],::coneactor->GetProperty()->GetColor()[2] };
                                if (ImGui::ColorEdit3("Color", color))
                                {
                                    ::coneactor->GetProperty()->SetColor(color[0], color[1], color[2]);
                                }
                                static float edgeColor[3] = { ::coneactor->GetProperty()->GetEdgeColor()[0],::coneactor->GetProperty()->GetEdgeColor()[1],::coneactor->GetProperty()->GetEdgeColor()[2] };
                                if (ImGui::ColorEdit3("EdgeColor", edgeColor))
                                {
                                    ::coneactor->GetProperty()->SetEdgeColor(edgeColor[0], edgeColor[1], edgeColor[2]);
                                }
                                static float vertexColor[3] = { ::coneactor->GetProperty()->GetVertexColor()[0],::coneactor->GetProperty()->GetVertexColor()[1],::coneactor->GetProperty()->GetVertexColor()[2] };
                                if (ImGui::ColorEdit3("VertexColor", vertexColor))
                                {
                                    ::coneactor->GetProperty()->SetVertexColor(vertexColor[0], vertexColor[1], vertexColor[2]);
                                }
                                static float coatColor[3] = { ::coneactor->GetProperty()->GetCoatColor()[0],::coneactor->GetProperty()->GetCoatColor()[1],::coneactor->GetProperty()->GetCoatColor()[2] };
                                if (ImGui::ColorEdit3("CoatColor", coatColor))
                                {
                                    ::coneactor->GetProperty()->SetCoatColor(coatColor[0], coatColor[1], coatColor[2]);
                                }
                                if (float opacity = ::coneactor->GetProperty()->GetOpacity(); ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "opacity = %.3f"))
                                {
                                    ::coneactor->GetProperty()->SetOpacity(opacity);
                                }
                                if (float v = ::coneactor->GetProperty()->GetLineWidth(); ImGui::SliderFloat("LineWidth", &v, 0.0f, 30.0f))
                                {
                                    ::coneactor->GetProperty()->SetLineWidth(v);
                                }
                                if (float v = ::coneactor->GetProperty()->GetPointSize(); ImGui::SliderFloat("PointSize", &v, 1.0f, 100.0f))
                                {
                                    ::coneactor->GetProperty()->SetPointSize(v);
                                }
                                if (float v = ::coneactor->GetProperty()->GetCoatStrength(); ImGui::SliderFloat("CoatStrength", &v, 0.0f, 1.0f))
                                {
                                    ::coneactor->GetProperty()->SetCoatStrength(v);
                                }
                                if (float v = ::coneactor->GetProperty()->GetCoatRoughness(); ImGui::SliderFloat("CoatRoughness", &v, 0.0f, 1.0f))
                                {
                                    ::coneactor->GetProperty()->SetCoatRoughness(v);
                                }
                                if (float v = ::coneactor->GetProperty()->GetMetallic(); ImGui::SliderFloat("Metallic", &v, 0.0f, 1.0f))
                                {
                                    ::coneactor->GetProperty()->SetMetallic(v);
                                }
                                ImGui::TreePop();
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Camera"))
                    {
                        ImGuiNs::cameraSetup(::renderer->GetActiveCamera());
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                
#if 0
                if (ImGui::CollapsingHeader("vtkImageData", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    int dims[3];
                    ::reader->GetOutput()->GetDimensions(dims);
                    ImGui::Text(fmt::format("Dimensions: {}", dims).c_str());
                }

                auto lookupmap = colorMap->GetLookupTable();
                double* pRange = lookupmap->GetRange();
                float min_ = pRange[0];
                float max_ = pRange[1];
                if (ImGui::DragFloatRange2("ScalarsToColorsRange", &min_, &max_, 1.f, 0.0f, 10000.0f, "Min: %lf", "Max: %lf"))
                {
                    lookupmap->SetRange(min_, max_);
                    ::colorMap->Update();
                }

                // slab mode
                {
                    const char* slabModeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
                    auto currentSlabMode = ::reslice->GetSlabMode();
                    if (ImGui::Combo("SlabMode", &currentSlabMode, slabModeText, IM_ARRAYSIZE(slabModeText)))
                    {
                        ::reslice->SetSlabMode(currentSlabMode);
                        ::colorMap->Update();
                    }
                }
                // thickness
                {
                    const char* items[] = { "1 mm", "5 mm", "10 mm", "15 mm", "100 mm", "1000 mm" };
                    int dataArray[] = { 1, 5, 10, 15, 100, 1000 };
                    static int currentIdx = -1;
                    if (ImGui::Combo("Thickness", &currentIdx, items, IM_ARRAYSIZE(items)))
                    {
                        const auto n = dataArray[currentIdx] / ::spacing[2];
                        ::reslice->SetSlabNumberOfSlices(n);
                        ::colorMap->Update();
                    }
                    auto numOfSlices = ::reslice->GetSlabNumberOfSlices();
                    if (ImGui::DragInt("SlabNumberOfSlices", &numOfSlices, 1, 1, 10000))
                    {
                        ::reslice->SetSlabNumberOfSlices(numOfSlices);
                        ::colorMap->Update();
                    }
                }
                // output
                if (ImGui::TreeNodeEx("Output", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    {
                        double myArray[3];
                        ::reslice->GetOutputSpacing(myArray);
                        if (ImGui::DragScalarN("Spacing", ImGuiDataType_Double, myArray, 3, .1f))
                        {
                            ::reslice->SetOutputSpacing(myArray);
                            ::colorMap->Update();
                        }
                    }
                    {
                        int myArray[6];
                        ::reslice->GetOutputExtent(myArray);
                        if (ImGui::DragScalarN("Extent", ImGuiDataType_S32, myArray, 6))
                        {
                            ::reslice->SetOutputExtent(myArray);
                            ::colorMap->Update();
                        }
                    }

                    ImGui::TreePop();
                }
                {
                    if (ImGui::TreeNodeEx("ResliceAxesDirectionCosines", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        double xyz[9];
                        ::reslice->GetResliceAxesDirectionCosines(xyz);
                        if (ImGui::DragScalarN("X", ImGuiDataType_Double, xyz, 3, .01f))
                        {
                            ::reslice->SetResliceAxesDirectionCosines(xyz);
                            ::colorMap->Update();
                        }
                        if (ImGui::DragScalarN("Y", ImGuiDataType_Double, xyz + 3, 3, .01f))
                        {
                            ::reslice->SetResliceAxesDirectionCosines(xyz);
                            ::colorMap->Update();
                        }
                        if (ImGui::DragScalarN("Z", ImGuiDataType_Double, xyz + 6, 3, .01f))
                        {
                            ::reslice->SetResliceAxesDirectionCosines(xyz);
                            ::colorMap->Update();
                        }

                        ImGui::TreePop();
                    }

                    double o[3];
                    ::reslice->GetResliceAxesOrigin(o);
                    if (ImGui::DragScalarN("ResliceAxesOrigin", ImGuiDataType_Double, o, 3, .1f))
                    {
                        ::reslice->SetResliceAxesOrigin(o);
                        ::colorMap->Update();
                    }
                }
                {
                    if (auto wrap = ::reslice->GetWrap(); ImGui::Button(fmt::format("Wrap: {}", wrap).c_str()))
                    {
                        ::reslice->SetWrap(!wrap);
                        ::reslice->Update();
                        ::colorMap->Update();
                    }
                    ImGui::SameLine();
                    if (auto mirror = ::reslice->GetMirror(); ImGui::Button(fmt::format("Mirror: {}", mirror).c_str()))
                    {
                        ::reslice->SetMirror(!mirror);
                        ::reslice->Update();
                        ::colorMap->Update();
                    }
                }
                if (ImGui::Button("ResetCamera"))
                {
                    ::renderer->ResetCamera();
                }
                if (ImGui::Button("AddMarker"))
                {
                    auto text = ::getTextActor();
                    text->SetInput(fmt::format("{}",::reslice->GetResliceAxesOrigin()[2]).c_str());
                    text->GetPositionCoordinate()->SetCoordinateSystemToWorld();
                    text->SetPosition(::actor->GetCenter());
                    ::renderer->AddActor(text);

                    static std::map<void*, vtkSmartPointer<vtkMatrix4x4>> actorMap;
                    auto myMat = vtkSmartPointer<vtkMatrix4x4>::New();
                    myMat->DeepCopy(::reslice->GetResliceAxes());
                    actorMap.emplace(text.GetPointer(), myMat);

                    auto callback = vtkSmartPointer<vtkCallbackCommand>::New();
                    callback->SetClientData(text.GetPointer());
                    callback->SetCallback([](vtkObject* caller, unsigned long eventId, void* clientData, void* callData)
                        {
                            if (vtkCommand::ModifiedEvent == eventId)
                            {
                                auto isMatrixEqual = [](const vtkMatrix4x4* matrix1, const vtkMatrix4x4* matrix2)
                                    {
                                        for (int i = 0; i < 4; i++)
                                        {
                                            for (int j = 0; j < 4; j++)
                                            {
                                                if (matrix1->GetElement(i, j) != matrix2->GetElement(i, j))
                                                {
                                                    return false;
                                                }
                                            }
                                        }
                                        return true;
                                    };
                                auto currMat = ::reslice->GetResliceAxes();
                                auto mat = actorMap[clientData];
                                auto text = reinterpret_cast<vtkTextActor*>(clientData);
                                text->SetVisibility(isMatrixEqual(currMat, mat));
                            }
                        });
                    ::reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, callback);
                }
#endif            
}
            ImGui::End();
        };
    uiDraw->SetCallback(uiDrawFunction);
    overlay->AddObserver(vtkDearImGuiInjector::ImGuiDrawEvent, uiDraw);
}