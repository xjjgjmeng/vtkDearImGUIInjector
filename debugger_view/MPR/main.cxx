#include "../IncludeAllInOne.h"

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

vtkSmartPointer<vtkResliceImageViewer> riw[3];
vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
vtkSmartPointer<vtkRenderer> renderer4;;
//vtkSmartPointer<vtkDistanceWidget> DistanceWidget[3];
//vtkSmartPointer<vtkResliceImageViewerMeasurements> ResliceMeasurements;
class vtkResliceCursorCallback : public vtkCommand
{
public:
    static vtkResliceCursorCallback* New() { return new vtkResliceCursorCallback; }

    void Execute(vtkObject* caller, unsigned long ev, void* callData) override
    {

        if (ev == vtkResliceCursorWidget::WindowLevelEvent || ev == vtkCommand::WindowLevelEvent ||
            ev == vtkResliceCursorWidget::ResliceThicknessChangedEvent)
        {
            // Render everything
            for (int i = 0; i < 3; i++)
            {
                this->RCW[i]->Render();
            }
            this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
            return;
        }

        vtkImagePlaneWidget* ipw = dynamic_cast<vtkImagePlaneWidget*>(caller);
        if (ipw)
        {
            double* wl = static_cast<double*>(callData);

            if (ipw == this->IPW[0])
            {
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[1])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[2]->SetWindowLevel(wl[0], wl[1], 1);
            }
            else if (ipw == this->IPW[2])
            {
                this->IPW[0]->SetWindowLevel(wl[0], wl[1], 1);
                this->IPW[1]->SetWindowLevel(wl[0], wl[1], 1);
            }
        }

        vtkResliceCursorWidget* rcw = dynamic_cast<vtkResliceCursorWidget*>(caller);
        if (rcw)
        {
            vtkResliceCursorLineRepresentation* rep =
                dynamic_cast<vtkResliceCursorLineRepresentation*>(rcw->GetRepresentation());
            // Although the return value is not used, we keep the get calls
            // in case they had side-effects
            rep->GetResliceCursorActor()->GetCursorAlgorithm()->GetResliceCursor();
            for (int i = 0; i < 3; i++)
            {
                vtkPlaneSource* ps = static_cast<vtkPlaneSource*>(this->IPW[i]->GetPolyDataAlgorithm());
                ps->SetOrigin(
                    this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetOrigin());
                ps->SetPoint1(
                    this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint1());
                ps->SetPoint2(
                    this->RCW[i]->GetResliceCursorRepresentation()->GetPlaneSource()->GetPoint2());

                // If the reslice plane has modified, update it on the 3D widget
                this->IPW[i]->UpdatePlacement();
            }
        }

        // Render everything
        for (int i = 0; i < 3; i++)
        {
            this->RCW[i]->Render();
        }
        this->IPW[0]->GetInteractor()->GetRenderWindow()->Render();
    }

    vtkResliceCursorCallback() {}
    vtkImagePlaneWidget* IPW[3];
    vtkResliceCursorWidget* RCW[3];
};
int main(int argc, char* argv[])
{
    //vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> renderWindow;
    vtkNew<vtkRenderWindowInteractor> iren;
    //renderWindow->SetMultiSamples(8);
    //renderWindow->AddRenderer(renderer);
    //iren->SetRenderWindow(renderWindow);

    // Create pipeline
    //vtkNew<vtkConeSource> coneSource;
    //coneSource->Update();

    //vtkNew<vtkPolyDataMapper> mapper;
    //mapper->SetInputConnection(coneSource->GetOutputPort());

    //vtkNew<vtkActor> actor;
    //actor->SetMapper(mapper);

    // Add the actors to the scene
    //renderer->AddActor(actor);

    auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName("D:/test_data/202110020082000");
    reader->Update();
    int imageDims[3];
    reader->GetOutput()->GetDimensions(imageDims);
    for (auto i = 0; i < 3; ++i)
    {
        riw[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
        
        riw[i]->SetRenderWindow(renderWindow);
        //riw[i]->SetSlice(50);
        //riw[i]->Render();
        riw[i]->SetupInteractor(iren);
        
        
        //riw[i]->GetRenderer()->ResetCameraClippingRange();
        
        auto rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
        riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());
        rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);
        riw[i]->SetInputData(reader->GetOutput());
        riw[i]->SetSliceOrientation(i);
        riw[i]->SetResliceModeToAxisAligned();

        
        //riw[i]->GetRenderWindow()->Print(std::cout);
        //riw[i]->SetRenderWindow(vtkSmartPointer<vtkRenderWindow>::New());
    }
    for (auto i = 0; i < 3; ++i)
    {
        renderWindow->AddRenderer(riw[i]->GetRenderer());
        riw[i]->GetRenderer()->ResetCamera();
    }
    riw[0]->GetRenderer()->SetViewport(0, 0.5, 0.5, 1);
    riw[1]->GetRenderer()->SetViewport(0.5, 0.5, 1, 1);
    riw[2]->GetRenderer()->SetViewport(0, 0, 0.5, 0.5);

    static auto picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);
    renderer4 = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer4);
    renderer4->SetViewport(0.5, 0, 1, 0.5);

    for (auto i = 0; i < 3; ++i)
    {
        auto w = vtkSmartPointer<vtkImagePlaneWidget>::New();
        planeWidget[i] = w;
        w->SetInteractor(iren);
        w->SetPicker(picker);
        w->RestrictPlaneToVolumeOn();
        double color[3]{};
        color[i] = 1;
        w->GetPlaneProperty()->SetColor(color);
        color[0] /= 4.;
        color[1] /= 4.;
        color[2] /= 4.;
        riw[i]->GetRenderer()->SetBackground(color);
        w->SetTexturePlaneProperty(vtkSmartPointer<vtkProperty>::New());
        w->TextureInterpolateOff();
        w->SetResliceInterpolateToLinear();
        w->SetInputConnection(reader->GetOutputPort());
        w->SetPlaneOrientation(i);
        w->SetSliceIndex(imageDims[i]/2);
        w->DisplayTextOn();
        w->SetDefaultRenderer(renderer4);
        w->SetWindowLevel(1358, -27);
        w->On();
        w->InteractionOn();
    }
    auto cbk = vtkSmartPointer<vtkResliceCursorCallback>::New();
    for (int i = 0; i < 3; ++i)
    {
        cbk->IPW[i] = planeWidget[i];
        cbk->RCW[i] = riw[i]->GetResliceCursorWidget();
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceAxesChangedEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::WindowLevelEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResliceThicknessChangedEvent, cbk);
        riw[i]->GetResliceCursorWidget()->AddObserver(vtkResliceCursorWidget::ResetCursorEvent, cbk);
        riw[i]->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, cbk);

        riw[i]->SetLookupTable(riw[0]->GetLookupTable());
        planeWidget[i]->GetColorMap()->SetLookupTable(riw[0]->GetLookupTable());
        planeWidget[i]->SetColorMap(riw[i]->GetResliceCursorWidget()->GetResliceCursorRepresentation()->GetColorMap());
    }

#if 0
    {
        static auto myviewer = vtkSmartPointer<vtkResliceImageViewer>::New();
        myviewer->SetInputData(reader->GetOutput());
        myviewer->SetRenderWindow(renderWindow);
        myviewer->SetSlice(50);
        myviewer->Render();
        myviewer->SetSliceOrientation(0);
        myviewer->SetupInteractor(iren);
        renderWindow->AddRenderer(myviewer->GetRenderer());
        myviewer->GetRenderer()->SetViewport(0, 0.5, 0.5, 1);
        myviewer->GetRenderer()->ResetCamera();
    }
    {
        static auto myviewer = vtkSmartPointer<vtkResliceImageViewer>::New();
        myviewer->SetInputData(reader->GetOutput());
        myviewer->SetRenderWindow(renderWindow);
        myviewer->SetSlice(50);
        myviewer->Render();
        myviewer->SetSliceOrientation(1);
        myviewer->SetupInteractor(iren);
        renderWindow->AddRenderer(myviewer->GetRenderer());
        myviewer->GetRenderer()->SetViewport(0.5, 0.5, 1, 1);
        myviewer->GetRenderer()->ResetCamera();
    }
#endif
    // Start rendering app
    //renderer->SetBackground(0.2, 0.3, 0.4);
    renderWindow->Render();

#if 0


    for (auto i = 0; i < 3; ++i)
    {
        riw[i] = vtkSmartPointer<vtkResliceImageViewer>::New();
        //riw[i]->GetRenderWindow()->Print(std::cout);
        //riw[i]->SetRenderWindow(vtkSmartPointer<vtkRenderWindow>::New());
    }
    riw[0]->GetRenderer()->SetViewport(0, 0.5, 0.5, 1);
    riw[1]->GetRenderer()->SetViewport(0.5, 0.5, 1, 1);
    riw[2]->GetRenderer()->SetViewport(0, 0, 0.5, 0.5);
    for (auto i : riw)
    {
        renderWindow->AddRenderer(i->GetRenderer());
    }

    for (int i = 0; i < 3; ++i)
    {
        auto* rep = vtkResliceCursorLineRepresentation::SafeDownCast(riw[i]->GetResliceCursorWidget()->GetRepresentation());
        riw[i]->SetResliceCursor(riw[0]->GetResliceCursor());
        rep->GetResliceCursorActor()->GetCursorAlgorithm()->SetReslicePlaneNormal(i);

        riw[i]->SetInputData(reader->GetOutput());
        riw[i]->SetSliceOrientation(i);
        riw[i]->SetResliceModeToAxisAligned();
    }
#endif

#if 0
    // Create a renderer, render window, and interactor
    renderer = vtkSmartPointer<vtkRenderer>::New();
    iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetMultiSamples(8);
    renderWindow->AddRenderer(renderer);
    iren->SetRenderWindow(renderWindow);

    reader = vtkSmartPointer<vtkDICOMImageReader>::New();
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
#endif

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
            //ImGui::SetNextWindowSize(ImVec2(450, 550), ImGuiCond_Once);
            ImGui::Begin("VTK");
            {
                // slab mode
                {
                    const char* slabModeText[] = { "RESLICE_AXIS_ALIGNED", "RESLICE_OBLIQUE" };
                    auto currentSlabMode = riw[0]->GetResliceMode();
                    if (ImGui::Combo("SlabMode", &currentSlabMode, slabModeText, IM_ARRAYSIZE(slabModeText)))
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            riw[i]->SetResliceMode(currentSlabMode);
                            riw[i]->GetRenderer()->ResetCamera();
                            riw[i]->Render();
                        }
                    }
                }
                {
                    if (ImGui::Button("Roll"))
                    {
                        renderer4->GetActiveCamera()->Roll(10);
                    }
                    if (ImGui::Button("Azimuth"))
                    {
                        renderer4->GetActiveCamera()->Azimuth(10);
                    }
                }
                {
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::Text("Azimuth"); ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##leftAzimuth", ImGuiDir_Left)) { renderer4->GetActiveCamera()->Azimuth(5);; }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##rightAzimuth", ImGuiDir_Right)) { renderer4->GetActiveCamera()->Azimuth(-5);; }
                    ImGui::PopButtonRepeat();

                    ImGui::Text("Roll"); ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##leftRoll", ImGuiDir_Left)) { renderer4->GetActiveCamera()->Roll(5);; }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##rightRoll", ImGuiDir_Right)) { renderer4->GetActiveCamera()->Roll(-5);; }
                    ImGui::PopButtonRepeat();
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
