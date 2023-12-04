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
      int lastPos[2];
      iren->GetLastEventPosition(lastPos);
      int currPos[2];
      iren->GetEventPosition(currPos);
      if (this->Slicing)
      {
          // Increment slice position by deltaY of mouse
          int deltaY = lastPos[1] - currPos[1];

          reslice->Update();
          double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
          vtkMatrix4x4* matrix = reslice->GetResliceAxes();
          // move the center point that we are slicing through
          double point[4];
          double center[4];
          point[0] = 0.0;
          point[1] = 0.0;
          point[2] = sliceSpacing * deltaY;
          fmt::print("point:{}\n", point[2]);
          point[3] = 1.0;
          matrix->MultiplyPoint(point, center);
          matrix->SetElement(0, 3, center[0]);
          matrix->SetElement(1, 3, center[1]);
          matrix->SetElement(2, 3, center[2]);
          colorMap->Update();
          iren->GetRenderWindow()->Render();
      }
  }

private:
  int Slicing=0;
};

int main(int argc, char* argv[])
{
    // Create a renderer, render window, and interactor
    vtkNew<vtkRenderer> renderer;
    iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetMultiSamples(8);
    renderWindow->AddRenderer(renderer);
    iren->SetRenderWindow(renderWindow);

#if 0
    // Create pipeline
    vtkNew<vtkConeSource> coneSource;
    coneSource->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(coneSource->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    // Add the actors to the scene
    renderer->AddActor(actor);
#else
    auto readerDcm = vtkSmartPointer<vtkDICOMImageReader>::New();
    readerDcm->SetDirectoryName("C:/Users/123/Desktop/180327-hxy/1.2.276.0.7230010.3.1.2.3994676664.11912.1522116132.845/1.2.276.0.7230010.3.1.3.3994676664.11912.1522116132.846");
    readerDcm->Update();
    //readerDcm->Print(std::cout);
    //img = readerDcm->GetOutput();

    int extent[6];
    double spacing[3];
    double origin[3];
    readerDcm->GetOutput()->GetExtent(extent);
    readerDcm->GetOutput()->GetSpacing(spacing);
    readerDcm->GetOutput()->GetOrigin(origin);
    //fmt::print("{} {} {}\n", extent, spacing, origin);

    double center[3];
    center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
    center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
    center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);

    static double axialElement[16] =
    {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };

    static auto resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
    resliceAxes->DeepCopy(axialElement);
    resliceAxes->SetElement(0, 3, center[0]);
    resliceAxes->SetElement(1, 3, center[1]);
    resliceAxes->SetElement(2, 3, center[2]);

    reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputConnection(readerDcm->GetOutputPort());
    reslice->SetOutputDimensionality(2);
    reslice->SetResliceAxes(resliceAxes);
    reslice->SetInterpolationModeToLinear();

    static auto colorTable = vtkSmartPointer<vtkLookupTable>::New();
    colorTable->SetRange(0, 1000);
    colorTable->SetValueRange(0.0, 1.0);
    colorTable->SetSaturationRange(0.0, 0.0);
    colorTable->SetRampToLinear();
    colorTable->Build();

    colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
    colorMap->SetLookupTable(colorTable);
    colorMap->SetInputConnection(reslice->GetOutputPort());
    colorMap->Update();

    static auto actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInputData(colorMap->GetOutput());
    renderer->AddActor(actor);
#endif

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
    //renderWindow->SetSize(1920, 1000);
#ifdef _WIN32
// 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, renderWindow->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
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
#ifndef ADOBE_IMGUI_SPECTRUM
                io.Fonts->AddFontFromMemoryCompressedBase85TTF(Karla_Regular_compressed_data_base85, 16);
                io.Fonts->AddFontDefault();
#else
                ImGui::Spectrum::LoadFont(18.0f);
                ImGui::Spectrum::StyleColorsSpectrum();
#endif
                auto& style = ImGui::GetStyle();
                style.ChildRounding = 8;
                style.FrameRounding = 8;
                style.GrabRounding = 8;
                style.PopupRounding = 8;
                style.ScrollbarRounding = 8;
                style.TabRounding = 8;
                style.WindowRounding = 8;
                style.FrameBorderSize = 1.f;
            }
        };
    uiSetup->SetCallback(uiSetupFunction);
    overlay->AddObserver(vtkDearImGuiInjector::ImGuiSetupEvent, uiSetup);
}

auto myfunc = [](vtkRenderWindowInteractor* interactor)
    {
        reslice->Update();
        double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
        auto matrix = reslice->GetResliceAxes();
        double point[4];
        double center[4];
        point[0] = 0;
        point[1] = 0;
        point[2] = sliceSpacing * 1;
        point[3] = 1;
        matrix->MultiplyPoint(point, center);
        matrix->SetElement(0, 3, center[0]);
        matrix->SetElement(1, 3, center[1]);
        matrix->SetElement(2, 3, center[2]);
        colorMap->Update();
        interactor->Render();
        interactor->GetRenderWindow()->Render();
    };

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
            if (ImGui::CollapsingHeader("vtkRenderWindow", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto rw = overlay_->Interactor->GetRenderWindow();
                ImGui::Text("MTime: %ld", rw->GetMTime());
                ImGui::Text("Name: %s", rw->GetClassName());
                if (ImGui::TreeNode("Capabilities"))
                {
                    ImGui::TextWrapped("OpenGL: %s", rw->ReportCapabilities());
                    ImGui::TreePop();
                }
            }
            if (ImGui::CollapsingHeader("vtkRenderWindowInteractor", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& iren = overlay_->Interactor;
                ImGui::Text("MTime: %ld", iren->GetMTime());
                ImGui::Text("Name: %s", iren->GetClassName());
                if (ImGui::TreeNode("Style"))
                {
                    auto styleBase = iren->GetInteractorStyle();
                    vtkInteractorObserver* iStyle = nullptr;
                    if (styleBase->IsA("vtkInteractorStyleSwitchBase"))
                    {
                        iStyle = vtkInteractorStyleSwitch::SafeDownCast(styleBase)->GetCurrentStyle();
                    }
                    else
                    {
                        iStyle = styleBase;
                    }
                    ImGui::Text("MTime: %ld", iStyle->GetMTime());
                    ImGui::Text("Name: %s", iStyle->GetClassName());
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Mouse"))
                {
                    int* xy = iren->GetEventPosition();
                    ImGui::Text("X: %d", xy[0]);
                    ImGui::Text("Y: %d", xy[1]);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Keyboard"))
                {
                    ImGui::Text("KeySym: %s", iren->GetKeySym());
                    ImGui::SameLine();
                    HelpMarker("VTK does not flush KeySym per frame.");
                    ImGui::Text("KeyCode: %c", iren->GetKeyCode());
                    ImGui::Text("Mods: %s %s %s", (iren->GetAltKey() ? "ALT" : " "),
                        (iren->GetControlKey() ? "CTRL" : " "), (iren->GetShiftKey() ? "SHIFT" : " "));
                    ImGui::TreePop();
                }
            }
#if 1
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Hold to repeat:");
            ImGui::SameLine();

            // Arrow buttons with Repeater
            static int counter = 0;
            float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { counter--; myfunc(overlay_->Interactor); }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { counter++;  myfunc(overlay_->Interactor); }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            ImGui::Text("%d", counter);

            const auto resliceAxes = reslice->GetResliceAxes();
            ImGui::Text(fmt::format("Origin: {:.2f} {:.2f} {:.2f}", resliceAxes->GetElement(0, 3), resliceAxes->GetElement(1, 3), resliceAxes->GetElement(2, 3)).c_str());

            auto lookupmap = colorMap->GetLookupTable();
            double* pRange = lookupmap->GetRange();
            float near_ = pRange[0];
            float far_ = pRange[1];
            if (ImGui::DragFloatRange2("ClippingRange", &near_, &far_, 1.f, 0.0f, 2000.0f, "Near: %lf", "Far: %lf"))
            {
                lookupmap->SetRange(near_, far_);
                //lookupmap->Build();
                //colorMap->Modified();
                colorMap->Update();
                //colorMap->SetLookupTable(lookupmap);
                renderWindow->Render();
            }
#endif
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
