#include <Windows.h>

#include <sstream>
#include <string>

#include "vtkDearImGuiInjector.h"

#include "vtkActor.h"
#include "vtkCallbackCommand.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkCameraOrientationRepresentation.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkConeSource.h"
#include "vtkInteractorStyle.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkImageReslice.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkRenderWindow.h"
#include "vtkImageMapToColors.h"
#include "vtkLookupTable.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkDICOMImageReader.h"
#include "vtkImageActor.h"
#include "vtkInteractorStyleImage.h"
#include "vtkfmt/core.h"
#include "vtkfmt/ranges.h"
#include "vtkfmt/format.h"

#ifdef ADOBE_IMGUI_SPECTRUM
#include "imgui_spectrum.h"
#endif
#include "imgui.h"                 // to draw custom UI
#include "vtkOpenGLRenderWindow.h" // needed to check if opengl is supported.

// Listens to vtkDearImGuiInjector::ImGuiSetupEvent
static void SetupUI(vtkDearImGuiInjector*);
// Listens to vtkDearImGuiInjector::ImGuiDrawEvent
static void DrawUI(vtkDearImGuiInjector*);
static void HelpMarker(const char* desc);

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------

vtkSmartPointer<vtkImageReslice> reslice;
vtkSmartPointer<vtkImageMapToColors> colorMap;
vtkSmartPointer<vtkRenderWindowInteractor> iren;
vtkSmartPointer<vtkRenderWindow> renderWindow;
vtkSmartPointer<vtkRenderer> renderer;
double spacing[3];

class MyStyle : public vtkInteractorStyleImage
{
public:
  static MyStyle* New() { return new MyStyle; }

  void 	OnLeftButtonDown() override
  {
      this->Slicing = 1;
  }

  void 	OnLeftButtonUp() override
  {
      this->Slicing = 0;
  }

  void 	OnMouseMove() override
  {
      if (this->Slicing)
      {
          int lastPos[2];
          iren->GetLastEventPosition(lastPos);
          int currPos[2];
          iren->GetEventPosition(currPos);
          // Increment slice position by deltaY of mouse
          const int deltaY = lastPos[1] - currPos[1];
          //reslice->Update();
          const double sliceSpacing = ::spacing[2];
          
          // move the center point that we are slicing through
          double point[4];
          point[0] = 0.0;
          point[1] = 0.0;
          point[2] = sliceSpacing * deltaY;
          //fmt::print("point:{}\n", point[2]);
          point[3] = 1.0;
          double center[4];
          vtkMatrix4x4* matrix = reslice->GetResliceAxes();
          matrix->MultiplyPoint(point, center);
          matrix->SetElement(0, 3, center[0]);
          matrix->SetElement(1, 3, center[1]);
          matrix->SetElement(2, 3, center[2]);
          ::colorMap->Update();
          ::renderWindow->Render();
      }
  }

private:
  int Slicing=0;
};

int main(int argc, char* argv[])
{
    // Create a renderer, render window, and interactor
    renderer = vtkSmartPointer<vtkRenderer>::New();
    iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetMultiSamples(8);
    renderWindow->AddRenderer(renderer);
    iren->SetRenderWindow(renderWindow);

    auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName("D:/test_data/202110020082000");
    reader->Update();

    int extent[6];
    double origin[3];
    reader->GetOutput()->GetExtent(extent);
    reader->GetOutput()->GetSpacing(::spacing);
    reader->GetOutput()->GetOrigin(origin);

    reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputConnection(reader->GetOutputPort());
    reslice->SetOutputDimensionality(2);
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

    auto actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInputData(colorMap->GetOutput());
    renderer->AddActor(actor);

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
                //io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf", 15.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
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

            ImGui::SetNextWindowBgAlpha(0.5);
            ImGui::SetNextWindowPos(ImVec2(5, 25), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(450, 550), ImGuiCond_Once);
            ImGui::Begin("VTK");
            {
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
            }
            ImGui::End();
        };
    uiDraw->SetCallback(uiDrawFunction);
    overlay->AddObserver(vtkDearImGuiInjector::ImGuiDrawEvent, uiDraw);
}

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
