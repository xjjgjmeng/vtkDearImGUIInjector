#include <ImGuiCommon.h>
#include <implot.h>

namespace {
    class vtkBoxCallback : public vtkCommand
    {
    public:
        static vtkBoxCallback* New()
        {
            return new vtkBoxCallback;
        }

        void Execute(vtkObject* caller, unsigned long, void*) override
        {
            auto boxWidget = vtkBoxWidget2::SafeDownCast(caller);
            vtkNew<vtkTransform> t;
            vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation())->GetTransform(t);
            this->m_actor->SetUserTransform(t);
        }

        vtkSmartPointer<vtkProp3D> m_actor;
    };
} // namespace

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> renPoly;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(renPoly);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(vtkns::getDicomDir());
    reader->Update();

    // 显示体数据用于对比
    {
        vtkNew<vtkRenderer> ren;
        ren->SetViewport(0.8, 0.5, 1, 1);
        renWin->AddRenderer(ren);

        vtkNew<vtkVolume> pVolume;
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
        pMapper->SetInputData(reader->GetOutput());
        pMapper->SetBlendModeToComposite();
        pVolume->SetMapper(pMapper);
        ::setupDefaultVolumeProperty(pVolume);
        ren->AddVolume(pVolume);
        ren->ResetCamera();
    }

    // 只能处理一张图？？
    vtkNew<vtkImageToPolyDataFilter> imgPolyFilter;
    imgPolyFilter->SetColorModeToLUT();
    {
        vtkNew<vtkScalarsToColors> lut;
        lut->SetRange(0, 255);
        imgPolyFilter->SetLookupTable(lut);
    }
    imgPolyFilter->SetInputData(reader->GetOutput());
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(imgPolyFilter->GetOutputPort());
    vtkNew<vtkActor> polydataActor;
    polydataActor->SetMapper(mapper);
    polydataActor->GetProperty()->SetColor(1, 0, 0);
    renPoly->AddActor(polydataActor);

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        static bool showPolydata = true;
        ImGui::Checkbox("ShowPolydata", &showPolydata);
        if (showPolydata)
        {
            ImGui::Begin("PolyData");
            vtkns::vtkObjSetup("FilterOutput", imgPolyFilter->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        }
        vtkns::vtkObjSetup("filter", imgPolyFilter, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("actor", polydataActor);
    };

    // Start rendering app
    renWin->Render(); // 非常重要！！

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    ImPlot::CreateContext(); //
    // 💉 the overlay.
    dearImGuiOverlay->Inject(iren);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    vtkNew<vtkCameraOrientationWidget> camManipulator;
    camManipulator->SetParentRenderer(renPoly);
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
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();
    return 0;
}