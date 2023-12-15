﻿#include <ImGuiCommon.h>

ImGuiNs::ExampleAppLog logView;

class MyStyle : public vtkInteractorStyleImage
{
public:
    static MyStyle* New();
    vtkTypeMacro(MyStyle, vtkInteractorStyleImage);

    void OnLeftButtonDown() override
    {
        this->m_pressed = true;
    }

    void OnLeftButtonUp() override
    {
        this->m_pressed = false;
        this->m_currLineSource = nullptr;
    }

    void OnMouseMove() override
    {
        if (this->m_pressed)
        {
            int currPos[2];
            this->GetInteractor()->GetEventPosition(currPos);
            double displayPos[] = { currPos[0], currPos[1] };
            double worldPos[3];
            double worldOrient[9];
            const auto pRenderer = this->GetInteractor()->FindPokedRenderer(currPos[0], currPos[1]);
            this->m_placer->ComputeWorldPosition(pRenderer, displayPos, worldPos, worldOrient);
            ::logView.Add(fmt::format("EventPosition: {}", currPos));
            ::logView.Add(fmt::format("worldPos: {}", worldPos));
            ::logView.Add(fmt::format("worldOrient: {}", worldOrient));
            const auto withinImage = this->m_placer->ValidateWorldPosition(worldPos);
            ::logView.Add(fmt::format("ValidateWorldPosition: {}", withinImage));

            // 其他方式计算世界坐标
            {
                // 0
                {
                    double worldPt[4];
                    vtkInteractorObserver::ComputeDisplayToWorld(pRenderer, currPos[0], currPos[1], 0, worldPt);
                    ::logView.Add(fmt::format("worldPt0: {}", worldPt));
                }
                // 1
                {
                    vtkNew<vtkCoordinate> coordinate;
                    coordinate->SetCoordinateSystemToDisplay();
                    coordinate->SetValue(currPos[0], currPos[1]);
                    auto worldPt = coordinate->GetComputedWorldValue(pRenderer);
                    ::logView.Add(fmt::format("worldPt1: {}", std::initializer_list{ worldPt[0],worldPt[1], worldPt[2] }));
                }
                // 2
                {
                    pRenderer->SetDisplayPoint(currPos[0], currPos[1], 0);
                    pRenderer->DisplayToWorld();
                    double worldPt[4];
                    pRenderer->GetWorldPoint(worldPt);
                    ::logView.Add(fmt::format("worldPt2: {}", worldPt));
                }
            }

            do
            {
                if (!withinImage && this->m_drawOntoImage)
                {
                    break;
                }

                if (!this->m_currLineSource)
                {
                    auto lineSource = vtkSmartPointer<vtkLineSource>::New();
                    this->m_currLineSource = lineSource; // 如果在上一句直接赋值会崩溃
                    {
                        vtkNew<vtkPoints> newPoints;
                        newPoints->InsertNextPoint(worldPos); // note: 前两个点是相同的
                        lineSource->SetPoints(newPoints);
                    }
                    vtkNew<vtkPolyDataMapper> mapper;
                    mapper->SetInputConnection(lineSource->GetOutputPort());
                    vtkNew<vtkActor> actor;
                    actor->GetProperty()->SetColor(1, 0, 0);
                    actor->SetMapper(mapper);
                    pRenderer->AddActor(actor);
                }
                this->m_currLineSource->GetPoints()->InsertNextPoint(worldPos);
                this->m_currLineSource->Modified();
                ::logView.Add(fmt::format("NumberOfPoints: {}", this->m_currLineSource->GetPoints()->GetNumberOfPoints()));
            } while (0);
        }
    }

    vtkImageActorPointPlacer* m_placer = nullptr;
    bool m_drawOntoImage = false;

private:
    vtkLineSource* m_currLineSource = nullptr;
    bool m_pressed = false;
};
vtkStandardNewMacro(MyStyle);

int main(int argc, char* argv[])
{
  vtkNew<vtkDICOMImageReader> reader;
#if 1
  reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
#else
  reader->SetFileName(ImguiVtkNs::getDicomFile());
#endif
  reader->Update();
#if 0
  vtkNew<vtkXMLImageDataWriter> writer;
  writer->SetInputData(reader->GetOutput());
  writer->SetFileName("slice.vti");
  writer->Write();
#endif

  vtkNew<vtkResliceImageViewer> viewer;

  auto ren = viewer->GetRenderer();
  auto renWin = viewer->GetRenderWindow();
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  viewer->SetInputData(reader->GetOutput());
  viewer->SetSlice(212);
  viewer->SetSliceOrientationToXY();
  viewer->GetRenderer()->ResetCamera();
  
  vtkNew<vtkImageActorPointPlacer> placer;
  placer->SetImageActor(viewer->GetImageActor());

  auto myStyle = vtkSmartPointer<MyStyle>::New();
  myStyle->m_placer = placer;
  iren->SetInteractorStyle(myStyle);

  ::imgui_render_callback = [&]
      {
          if (ImGui::TreeNode("Log"))
          {
              logView.Draw();
              ImGui::TreePop();
          }
          if (ImGui::TreeNodeEx("vtkImageActor", ImGuiTreeNodeFlags_DefaultOpen))
          {
              ImGuiNs::vtkObjSetup(viewer->GetImageActor());
              ImGui::TreePop();
          }
          if (ImGui::TreeNodeEx("Camera"))
          {
              ImGuiNs::vtkObjSetup(ren->GetActiveCamera());
              ImGui::TreePop();
          }
          if (ImGui::TreeNode("vtkResliceImageViewer"))
          {
              if (int v[3]; viewer->GetSliceRange(v), v[2] = viewer->GetSlice(), ImGui::SliderInt("Slice", &v[2], v[0], v[1]))
              {
                  viewer->SetSlice(v[2]);
              }
              ImGui::TreePop();
          }
          ImGui::Checkbox(u8"只能绘制到影像上", &myStyle->m_drawOntoImage);
      };

  // Start rendering app
  ren->SetBackground(0.2, 0.3, 0.4);
  renWin->Render(); // 非常重要！！

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}