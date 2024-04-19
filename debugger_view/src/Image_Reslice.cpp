#include <ImGuiCommon.h>
#include <PolyDataHelper.h>

#define USE_MAP_TO_COLORS 0

vtkSmartPointer<vtkImageReslice> reslice;
#if 1 == USE_MAP_TO_COLORS
vtkSmartPointer<vtkImageMapToColors> colorMap;
#endif
double spacing[3];
bool calcRelice2Vr = false;

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
#if 1 == USE_MAP_TO_COLORS
        ::colorMap->Update();
#else
        ::reslice->Update(); // 没有此句会输出的都是二维
#endif
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
    vtkns::labelWorldZero(ren);

    auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(vtkns::getDicomDir());
    reader->Update();

    int extent[6];
    double origin[3];
    reader->GetOutput()->GetExtent(extent);
    reader->GetOutput()->GetSpacing(::spacing);
    reader->GetOutput()->GetOrigin(origin);

    reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputConnection(reader->GetOutputPort());
    reslice->SetOutputDimensionality(2);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, reader->GetOutput(), false)->GetProperty()->SetColor(1., 1., 0.);
    // 将reslice得到的image用线框动态表示出来
    {
        auto actor = vtkns::genImgOutline(ren, reslice->GetOutput(), true);
        actor->GetProperty()->SetColor(1., 0., 0.);
        actor->GetProperty()->SetLineWidth(5);
    }

    if (3 == ::reslice->GetOutputDimensionality())
    {
        // 将切割出来的体数据渲染出来
        vtkNew<vtkVolume> pVolume;
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
        pMapper->SetInputConnection(::reslice->GetOutputPort());
        pMapper->SetBlendModeToComposite();
        pVolume->SetMapper(pMapper);
        ::setupDefaultVolumeProperty(pVolume);
        vtkNew<vtkRenderer> ren;
        ren->AddVolume(pVolume);
        ren->SetViewport(0.7, 0.5, 1., 1.);
        ren->SetBackground(0, 1, 1);
        ren->SetBackgroundAlpha(0.2);
        vtkns::genImgOutline(ren, reader->GetOutput(), false)->GetProperty()->SetColor(1., 1., 0.);
        vtkns::genImgOutline(ren, reslice->GetOutput(), true)->GetProperty()->SetColor(1., 0., 0.);
        rw->AddRenderer(ren);
}
    else
    {
        // 显示一个完整图用于比较
        vtkNew<vtkImageActor> actor;
        actor->SetInputData(reader->GetOutput());
        vtkNew<vtkRenderer> ren;
        ren->AddActor(actor);
        ren->SetViewport(0.8, 0.8, 1., 1.);
        rw->AddRenderer(ren);
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
#if 0
                    if (auto mat = vtkMatrix4x4::SafeDownCast(caller))
                    {
                        double src[4]{ 0,0,0,1 };
                        double dst[4];
                        mat->MultiplyPoint(src, dst);
                        vtkNs::makePoints({ {dst[0], dst[1], dst[2]} }, actor);
                    }
#elif 1
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
#else
                    if (auto mat = vtkMatrix4x4::SafeDownCast(caller))
                    {
                        vtkNew<vtkPlaneSource> src;
                        src->SetOrigin(0, 0, 0);
                        src->SetPoint1(13, 0, 0);
                        src->SetPoint2(0, 13, 0);
                        src->SetCenter(mat->GetElement(0, 3), mat->GetElement(1, 3), mat->GetElement(2, 3));
                        src->SetNormal(mat->GetElement(0, 2), mat->GetElement(1, 2), mat->GetElement(2, 2));
                        // src->Update(); 使用GetOutputPort就不需要Update
                        vtkNew<vtkPolyDataMapper> mapper;
                        mapper->SetInputConnection(src->GetOutputPort());
                        actor->SetMapper(mapper);
                    }
#endif
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

#if 1 == USE_MAP_TO_COLORS
    colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
    {
        auto colorTable = vtkSmartPointer<vtkLookupTable>::New();
        colorTable->SetRange(0, 1000);
        colorTable->SetValueRange(0.0, 1.0);
        colorTable->SetSaturationRange(0.0, 0.0);
        colorTable->SetRampToLinear();
        colorTable->Build();
        colorMap->SetLookupTable(colorTable);
    }
    colorMap->SetInputConnection(reslice->GetOutputPort());
    colorMap->Update();
#endif

    auto actor = vtkSmartPointer<vtkImageActor>::New();
#if 1 == USE_MAP_TO_COLORS
    actor->SetInputData(colorMap->GetOutput());
#else
    actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
#endif
    ren->AddActor(actor);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("vtkImageData", reader->GetOutput());
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
#if 1 == USE_MAP_TO_COLORS
        auto lookupmap = colorMap->GetLookupTable();
        double* pRange = lookupmap->GetRange();
        float min_ = pRange[0];
        float max_ = pRange[1];
        if (ImGui::DragFloatRange2("ScalarsToColorsRange", &min_, &max_, 1.f, 0.0f, 10000.0f, "Min: %lf", "Max: %lf"))
        {
            lookupmap->SetRange(min_, max_);
            ::colorMap->Update();
        }
#endif

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