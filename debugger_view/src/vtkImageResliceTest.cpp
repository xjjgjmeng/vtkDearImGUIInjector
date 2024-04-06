#include <ImGuiCommon.h>
#include <PolyDataHelper.h>

namespace
{
    auto getTextActor()
    {
        auto text = vtkSmartPointer<vtkTextActor>::New();
        text->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
        text->GetTextProperty()->SetFontFile("C:/Windows/Fonts/simhei.ttf");
        text->GetTextProperty()->SetColor(0, 1, 0);
        text->GetTextProperty()->SetOpacity(0.8);
        text->GetTextProperty()->SetBackgroundColor(1, 0, 0);
        text->GetTextProperty()->SetBackgroundOpacity(0.5);
        text->GetTextProperty()->SetFontSize(18);
        text->GetTextProperty()->SetJustification(VTK_TEXT_RIGHT);
        //text->GetPositionCoordinate()->SetCoordinateSystemToWorld();
        return text;
    }
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
#define USE_MAP_TO_COLORS 0

vtkSmartPointer<vtkImageReslice> reslice;
#if 1 == USE_MAP_TO_COLORS
vtkSmartPointer<vtkImageMapToColors> colorMap;
#endif
vtkSmartPointer<vtkRenderWindowInteractor> iren;
vtkSmartPointer<vtkRenderWindow> renderWindow;
double spacing[3];
bool addActorOnPress = false;

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static MyStyle* New() { return new MyStyle; }

  void OnLeftButtonDown() override
  {
      int eventPt[2];
      this->GetInteractor()->GetEventPosition(eventPt);

      if (this->GetInteractor()->FindPokedRenderer(eventPt[0], eventPt[1]) == this->m_renderer && ::addActorOnPress)
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

          this->m_placer->SetProjectionNormalToOblique();
          {
              vtkNew<vtkPlane> plane;
              plane->SetOrigin(0, 0, 0);
              plane->SetNormal(0, 0, 1);
              this->m_placer->SetObliquePlane(plane);
          }

          double worldPt_img[3];
          {
              double worldOrient[9];
              double eventPt_d[] = { eventPt[0], eventPt[1] };
              this->m_placer->ComputeWorldPosition(this->m_renderer, eventPt_d, worldPt_img, worldOrient);
          }

          double worldPt_original[4];
          {
              double pt[] = { worldPt_img[0], worldPt_img[1], worldPt_img[2], 1 };
              ::reslice->GetResliceAxes()->MultiplyPoint(pt, worldPt_original);
          }

          ::getLogView()->Add(fmt::format("worldPt_img: {::.2f}", worldPt_img));
          ::getLogView()->Add(fmt::format("worldPt_original: {::.2f}\n", worldPt_original));
          vtkNs::makePoints({ {worldPt_img[0], worldPt_img[1], worldPt_img[2]}}, actor_image);
          vtkNs::makePoints({ {worldPt_original[0], worldPt_original[1], worldPt_original[2]}}, actor_original);
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
          iren->GetLastEventPosition(lastPos);
          int currPos[2];
          iren->GetEventPosition(currPos);
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
        ::renderWindow->Render();
    }

public:
    vtkRenderer* m_renderer = nullptr;

private:
  bool m_slicing = false;
  vtkNew<vtkBoundedPlanePointPlacer> m_placer;
};

int main(int argc, char* argv[])
{
    // Create a renderer, render window, and interactor
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetMultiSamples(8);
    renderWindow->AddRenderer(renderer);
    iren->SetRenderWindow(renderWindow);

    vtkns::labelWorldZero(renderer);

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
    vtkns::genImgOutline(renderer, reader->GetOutput())->GetProperty()->SetColor(1., 1., 0.);
    // 将reslice得到的image用线框动态表示出来
    vtkns::genImgOutlineOnChanged(renderer, reslice->GetOutput())->GetProperty()->SetColor(1., 0., 0.);

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
        vtkns::genImgOutline(ren, reader->GetOutput())->GetProperty()->SetColor(1., 1., 0.);
        vtkns::genImgOutlineOnChanged(ren, reslice->GetOutput())->GetProperty()->SetColor(1., 0., 0.);
        renderWindow->AddRenderer(ren);
}
    else
    {
        // 显示一个完整图用于比较
        vtkNew<vtkImageActor> actor;
        actor->SetInputData(reader->GetOutput());
        vtkNew<vtkRenderer> ren;
        ren->AddActor(actor);
        ren->SetViewport(0.8, 0.8, 1., 1.);
        renderWindow->AddRenderer(ren);
    }

    {
        // ResliceAxesOrigin设置reslice坐标系的原点在世界坐标系中的位置，生成的slice的origin是相对这个点而言的
        // 如果调用了SetOutputOrigin，则输出的slice的origin只被此接口控制
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
                    auto mat = vtkMatrix4x4::SafeDownCast(caller);
#if 0
                    double src[4]{ 0,0,0,1 };
                    double dst[4];
                    mat->MultiplyPoint(src, dst);
                    vtkNs::makePoints({ {dst[0], dst[1], dst[2]} }, reinterpret_cast<vtkActor*>(clientdata));
#else
                    vtkNew<vtkPlaneSource> src;
                    src->SetOrigin(0, 0, 0);
                    src->SetPoint1(13, 0, 0);
                    src->SetPoint2(0, 13, 0);
                    src->SetCenter(mat->GetElement(0, 3), mat->GetElement(1, 3), mat->GetElement(2, 3));
                    src->SetNormal(mat->GetElement(0, 2), mat->GetElement(1, 2), mat->GetElement(2, 2));
                    // src->Update(); 使用GetOutputPort就不需要Update
                    vtkNew<vtkPolyDataMapper> mapper;
                    mapper->SetInputConnection(src->GetOutputPort());
                    reinterpret_cast<vtkActor*>(clientdata)->SetMapper(mapper);
#endif
                };
            vtkNew<vtkCallbackCommand> pCC;
            pCC->SetCallback(f);
            vtkNew<vtkActor> actor;
            actor->GetProperty()->SetPointSize(12);
            actor->GetProperty()->SetColor(1., 0., 0.);
            renderer->AddActor(actor);
            pCC->SetClientData(actor);
            ::reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, pCC);
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
    reslice->SetOutputOrigin(0, 0, 0); // 指定了输出的数据的extent(0,0,0)的世界坐标
    reslice->SetOutputExtent(0,400,0,400,0,400); // 指定了数据数据extent的范围，可以从0开始，也可以大于或小于0
    //reslice->SetOutputSpacing(1., 1., 1.); // 指定了数据点之间的世界坐标中的距离
    reslice->SetOutputSpacing(::spacing);
    ::reslice->Update(); // 没有此句的话在一开始不能显示三维线框
    // 某个voxel在世界中的位置: origin指定的世界点 + ijk * spacing指定的间距

    // 总结:
    // 在输出数据为二维的时候:
    // 根据ResliceAxes截取一个slice，将slice放置在XY平面上，将ResliceAxesOrigin对应的voxel或ijk和世界坐标的(0,0,0)对其
    // 再根据设置的OutputOrigin，OutputExtent（和OutputSpacing）在这个平面上滑动选取想要的区域
    // OutputExtent（和OutputSpacing）确定最终区域的大小，OutputOrigin确定这个区域（ijk为0，0，0）的起点

    // 再总结：
    // 将坐标系放置在指定的origin位置
    // xoy应该可以穿过imagedata，不然无输出
    // 固定output的origin和extent不变，修改ResliceAxes的origin的x和y，发现黑窗在世界中不动，image在黑窗口中游走
    // 固定ResliceAxes的origin，修改output的origin，发现黑窗在世界中游走，可以看到image不同部分
    // 固定ResliceAxes的origin，修改output的extent，发现黑窗在世界扩大或缩小，可以看到image的更多或更少
    // output的origin是相对于新坐标系的，把新坐标系的origin处看作（0，0，0），但最后在原始的坐标系中也是一样的
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
    renderer->AddActor(actor);

    ::pWindow = renderWindow;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("vtkImageData", reader->GetOutput());
        static bool showResliceOutput = false;
        ImGui::Checkbox("ShowResliceOutput", &showResliceOutput);
        ImGui::Checkbox("addActorOnPress", &::addActorOnPress);
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
            renderer->ResetCamera();
        }

        if (ImGui::Button("AddMarker"))
        {
            auto text = ::getTextActor();
            text->SetInput(fmt::format("{}", ::reslice->GetResliceAxesOrigin()[2]).c_str());
            text->GetPositionCoordinate()->SetCoordinateSystemToWorld();
            text->SetPosition(actor->GetCenter());
            renderer->AddActor(text);

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
    renderer->SetBackground(0.2, 0.3, 0.4);
    renderWindow->Render();

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
    auto style = vtkSmartPointer<MyStyle>::New();
    style->m_renderer = renderer;
    iren->SetInteractorStyle(style);
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}