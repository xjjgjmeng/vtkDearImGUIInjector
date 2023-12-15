#include "../IncludeAllInOne.h"
#include <ImGuiCommon.h>

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

vtkSmartPointer<vtkActor> coneActor;
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

  coneActor = vtkSmartPointer<vtkActor>::New();
  coneActor->SetMapper(mapper);

  // Add the actors to the scene
  renderer->AddActor(coneActor);

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
                        ImGuiNs::vtkObjSetup(::coneActor);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Camera"))
                    {
                        ImGuiNs::vtkObjSetup(::renderer->GetActiveCamera());
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Renderer"))
                    {
                        ImGuiNs::vtkObjSetup(::renderer);
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }         
            }
            ImGui::End();
        };
    uiDraw->SetCallback(uiDrawFunction);
    overlay->AddObserver(vtkDearImGuiInjector::ImGuiDrawEvent, uiDraw);
}