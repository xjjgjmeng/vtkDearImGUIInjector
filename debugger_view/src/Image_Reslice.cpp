#include <ImGuiCommon.h>
#include <PolyDataHelper.h>

#define SHOW_NULL 0
#define SHOW_CENTER_POINTER 1
#define SHOW_RESLICE_PLANE 2
#define SHOW_IMAGE_MESH 3

vtkSmartPointer<vtkImageReslice> reslice;
double spacing[3];
bool calcRelice2Vr = false;
int currHelperActor = SHOW_NULL;

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static MyStyle* New() { return new MyStyle; }

  void OnLeftButtonDown() override
  {
      int eventPt[2];
      this->GetInteractor()->GetEventPosition(eventPt);

      if (::calcRelice2Vr && (this->GetInteractor()->FindPokedRenderer(eventPt[0], eventPt[1]) == this->m_renderer))
      {
          vtkNew<vtkActor> actor_image;
          actor_image->GetProperty()->SetPointSize(12);
          actor_image->GetProperty()->SetColor(0., 1., 0.);
          actor_image->GetProperty()->SetOpacity(1.);
          this->m_renderer->AddActor(actor_image);

          vtkNew<vtkActor> actor_original;
          actor_original->GetProperty()->SetPointSize(12);
          actor_original->GetProperty()->SetColor(0,0,1);
          this->m_renderer->AddActor(actor_original);
          // 设置origin和normal
          this->m_placer->SetProjectionNormalToOblique();
          {
              vtkNew<vtkPlane> plane;
              plane->SetOrigin(0, 0, 0);
              plane->SetNormal(0, 0, 1);
              this->m_placer->SetObliquePlane(plane);
          }
          // 获取点击在reslice上的位置
          double worldPt_img[3];
          {
              double worldOrient[9];
              double eventPt_d[] = { eventPt[0], eventPt[1] };
              this->m_placer->ComputeWorldPosition(this->m_renderer, eventPt_d, worldPt_img, worldOrient);
          }
          // 转化为原始图像上的位置
          double worldPt_original[4];
          {
              double pt[] = { worldPt_img[0], worldPt_img[1], worldPt_img[2], 1 };
              ::reslice->GetResliceAxes()->MultiplyPoint(pt, worldPt_original);
          }

          ::getLogView()->Add(fmt::format("worldPt_img: {::.2f}", worldPt_img));
          ::getLogView()->Add(fmt::format("worldPt_original: {::.2f}\n", worldPt_original));
          vtkns::makePoints({ {worldPt_img[0], worldPt_img[1], worldPt_img[2]}}, actor_image);
          vtkns::makePoints({ {worldPt_original[0], worldPt_original[1], worldPt_original[2]}}, actor_original);
      }

      __super::OnLeftButtonDown();
  }

  void OnRightButtonDown() override
  {
      int eventPt[2];
      this->GetInteractor()->GetEventPosition(eventPt);

      if (this->GetInteractor()->FindPokedRenderer(eventPt[0], eventPt[1]) == this->m_renderer)
      {
          this->m_slicing = true;
      }
      else
      {
          __super::OnRightButtonDown();
      }
  }

  void OnRightButtonUp() override
  {
      this->m_slicing = false;
      __super::OnRightButtonUp();
  }

  void OnMouseMove() override
  {
      if (this->m_slicing)
      {
          int lastPos[2];
          this->Interactor->GetLastEventPosition(lastPos);
          int currPos[2];
          this->Interactor->GetEventPosition(currPos);
          // Increment slice position by deltaY of mouse
          resliceImg(lastPos[1] - currPos[1]);
      }
      else
      {
          __super::OnMouseMove();
      }
  }

  void OnMouseWheelForward() override
  {
      resliceImg(-1);
  }

  void OnMouseWheelBackward() override
  {
      resliceImg(1);
  }

private:
    void resliceImg(const int factor)
    {
        // move the center point that we are slicing through
        double point[4];
        point[0] = 0.0;
        point[1] = 0.0;
        point[2] = ::spacing[2] * factor;
        point[3] = 1.0;
        double center[4];
        vtkMatrix4x4* matrix = ::reslice->GetResliceAxes();
        matrix->MultiplyPoint(point, center);
        matrix->SetElement(0, 3, center[0]);
        matrix->SetElement(1, 3, center[1]);
        matrix->SetElement(2, 3, center[2]);
        ::reslice->Update(); // 没有此句会输出的都是二维
        this->Interactor->Render();
    }

public:
    vtkRenderer* m_renderer = nullptr;

private:
  bool m_slicing = false;
  vtkNew<vtkBoundedPlanePointPlacer> m_placer;
};

int main()
{
    SETUP_WINDOW
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    int extent[6];
    double origin[3];
    img->GetExtent(extent);
    img->GetSpacing(::spacing);
    img->GetOrigin(origin);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);
    // 将reslice得到的image用线框动态表示出来
    {
        auto actor = vtkns::genImgOutline(ren, reslice->GetOutput(), true);
        actor->GetProperty()->SetColor(1., 0., 0.);
        actor->GetProperty()->SetLineWidth(5);
    }

    if (3 == ::reslice->GetOutputDimensionality())
    {
        // 将切割出来的体数据渲染出来
        vtkns::genVR(ren, reslice->GetOutput(), false, true);
    }

    {
#if 1
        auto resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
        {
            const double axialElement[16] =
            {
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1
            };
            resliceAxes->DeepCopy(axialElement);
        }
        // 设置新坐标系的原点，单位是世界坐标
        {
            double center[3];
            center[0] = origin[0] + (extent[0]+extent[1])*0.5*spacing[0];
            center[1] = origin[1] + (extent[2]+extent[3])*0.5*spacing[1];
            center[2] = origin[2] + (extent[4]+extent[5])*0.5*spacing[2];
            resliceAxes->SetElement(0, 3, center[0]);
            resliceAxes->SetElement(1, 3, center[1]);
            resliceAxes->SetElement(2, 3, center[2]);
        }
        // 设置用于切割的坐标系
        reslice->SetResliceAxes(resliceAxes);
        // 变换矩阵变化的的时候显示最新的切面origin
        {
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                {
                    auto actor = reinterpret_cast<vtkActor*>(clientdata);
                    switch (::currHelperActor)
                    {
                    case SHOW_CENTER_POINTER:
                        if (auto mat = vtkMatrix4x4::SafeDownCast(caller))
                        {
                            double src[4]{ 0,0,0,1 };
                            double dst[4];
                            mat->MultiplyPoint(src, dst);
                            vtkns::makePoints({ {dst[0], dst[1], dst[2]} }, actor);
                        }
                        break;
                    case SHOW_IMAGE_MESH:
                        if (auto img = vtkImageData::SafeDownCast(caller))
                        {
                            vtkNew<vtkDataSetMapper> mapper;
                            vtkNew<vtkImageData> data;
                            data->SetOrigin(img->GetOrigin());
                            data->SetSpacing(img->GetSpacing());
                            data->SetExtent(img->GetExtent());
                            mapper->SetInputData(data);
                            actor->SetMapper(mapper);
                            actor->GetProperty()->SetRepresentationToWireframe();
                            actor->GetProperty()->SetColor(0, 1, 0);
                            actor->GetProperty()->SetOpacity(.1);
                            actor->SetPosition(0, 0, 1); // 设置偏移方便看到后面的图像
                        }
                        break;
                    case SHOW_RESLICE_PLANE:
                        if (auto mat = vtkMatrix4x4::SafeDownCast(caller))
                        {
                            vtkNew<vtkPlaneSource> src;
                            src->SetOrigin(0, 0, 0);
                            src->SetPoint1(33, 0, 0);
                            src->SetPoint2(0, 33, 0);
                            src->SetCenter(mat->GetElement(0, 3), mat->GetElement(1, 3), mat->GetElement(2, 3));
                            src->SetNormal(mat->GetElement(0, 2), mat->GetElement(1, 2), mat->GetElement(2, 2));
                            // src->Update(); 使用GetOutputPort就不需要Update
                            vtkNew<vtkPolyDataMapper> mapper;
                            mapper->SetInputConnection(src->GetOutputPort());
                            actor->SetMapper(mapper);
                        }
                        break;
                    default:
                        break;
                    }
                };
            vtkNew<vtkCallbackCommand> pCC;
            pCC->SetCallback(f);
            vtkNew<vtkActor> actor;
            actor->GetProperty()->SetPointSize(12);
            actor->GetProperty()->SetColor(1., 0., 0.);
            ren->AddActor(actor);
            pCC->SetClientData(actor);
            ::reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, pCC);
            ::reslice->GetOutput()->AddObserver(vtkCommand::ModifiedEvent, pCC);
        }
#else
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        const double o[3] = { 0,0,0 };
        ::reslice->SetResliceAxesDirectionCosines(x, y, z);
        ::reslice->SetResliceAxesOrigin(o);
#endif
    }
    reslice->SetInterpolationModeToLinear();
    reslice->SetOutputOrigin(0, 0, 0);
    reslice->SetOutputExtent(0,400,0,400,0,400);
    reslice->SetOutputSpacing(::spacing);
    ::reslice->Update(); // 没有此句的话在一开始不能显示三维线框

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
    ren->AddActor(actor);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        {
            ImGui::Text("HelperActor"); ImGui::SameLine();
            ImGui::RadioButton("NULL", &currHelperActor, SHOW_NULL); ImGui::SameLine();
            ImGui::RadioButton("CENTER_POINTER", &currHelperActor, SHOW_CENTER_POINTER); ImGui::SameLine();
            ImGui::RadioButton("RESLICE_PLANE", &currHelperActor, SHOW_RESLICE_PLANE); ImGui::SameLine();
            ImGui::RadioButton("IMAGE_MESH", &currHelperActor, SHOW_IMAGE_MESH);
        }
        vtkns::vtkObjSetup("OriginalImageData", img);
        static bool showResliceOutput = false;
        ImGui::Checkbox("ShowResliceOutput", &showResliceOutput);
        ImGui::Checkbox("Reslice2Vr", &::calcRelice2Vr); ImGui::SameLine(); vtkns::HelpMarker(u8"左键点击时将被点击的reslice上的位置映射到原始的VR空间并用点标识");
        if (showResliceOutput)
        {
            ImGui::Begin("ResliceOutput");
            vtkns::vtkObjSetup("ImageData", reslice->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        }
        vtkns::vtkObjSetup("vtkImageActor", actor);
        vtkns::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);

        if (ImGui::Button("ResetCamera"))
        {
            ren->ResetCamera();
        }

        if (ImGui::Button("AddMarker"))
        {
            auto text = vtkns::genTextActor();
            text->SetInput(fmt::format("{}", ::reslice->GetResliceAxesOrigin()[2]).c_str());
            text->GetPositionCoordinate()->SetCoordinateSystemToWorld();
            text->SetPosition(actor->GetCenter());
            ren->AddActor(text);

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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    auto style = vtkSmartPointer<MyStyle>::New();
    style->m_renderer = ren;
    rwi->SetInteractorStyle(style);
    rwi->EnableRenderOff();
    rwi->Start();
}