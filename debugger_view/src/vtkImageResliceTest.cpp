#include <ImGuiCommon.h>

#ifdef ADOBE_IMGUI_SPECTRUM
#include "imgui_spectrum.h"
#endif
#include "imgui.h"                 // to draw custom UI
#include "vtkOpenGLRenderWindow.h" // needed to check if opengl is supported.

static void HelpMarker(const char* desc);

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

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static MyStyle* New() { return new MyStyle; }

  void OnLeftButtonDown() override
  {
      this->m_slicing = true;
  }

  void OnLeftButtonUp() override
  {
      this->m_slicing = false;
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
#endif
        ::renderWindow->Render();
    }

private:
  bool m_slicing = false;
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

    ImguiVtkNs::labelWorldZero(renderer);

    auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();

    // 显示一个完整图用于比较
    {
        vtkNew<vtkImageActor> actor;
        actor->SetInputData(reader->GetOutput());
        vtkNew<vtkRenderer> ren;
        ren->AddActor(actor);
        ren->SetViewport(0.8, 0.8, 1., 1.);
        renderWindow->AddRenderer(ren);
    }

    int extent[6];
    double origin[3];
    reader->GetOutput()->GetExtent(extent);
    reader->GetOutput()->GetSpacing(::spacing);
    reader->GetOutput()->GetOrigin(origin);

    reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputConnection(reader->GetOutputPort());
    reslice->SetOutputDimensionality(2);
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
        {
            double center[3];
            center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
            center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
            center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
            resliceAxes->SetElement(0, 3, center[0]);
            resliceAxes->SetElement(1, 3, center[1]);
            resliceAxes->SetElement(2, 3, center[2]);
        }
        reslice->SetResliceAxes(resliceAxes);
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
    reslice->SetOutputExtent(0,100,0,100,0,100); // 指定了数据数据extent的范围，可以从0开始，也可以大于或小于0
    reslice->SetOutputSpacing(1., 1., 1.); // 指定了数据点之间的世界坐标中的距离
    // 某个voxel在世界中的位置: origin指定的世界点 + ijk * spacing指定的间距

    // 总结:
    // 在输出数据为二维的时候:
    // 根据ResliceAxes截取一个slice，将slice放置在XY平面上，将ResliceAxesOrigin对应的voxel或ijk和世界坐标的(0,0,0)对其
    // 再根据设置的OutputOrigin，OutputExtent（和OutputSpacing）在这个平面上滑动选取想要的区域
    // OutputExtent（和OutputSpacing）确定最终区域的大小，OutputOrigin确定这个区域（ijk为0，0，0）的起点
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

    // 将reslice得到的image用线框表示出来
    ImguiVtkNs::genImgOutlineOnChanged(renderer, reslice->GetOutput());

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
        ImGuiNs::vtkObjSetup("vtkImageData", reader->GetOutput());
        ImGuiNs::vtkObjSetup("vtkImageData_New", reslice->GetOutput());
        ImGuiNs::vtkObjSetup("vtkImageActor", actor);
        ImGuiNs::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);
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
    ImguiVtkNs::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    ImguiVtkNs::DrawUI(dearImGuiOverlay);
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
    iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}