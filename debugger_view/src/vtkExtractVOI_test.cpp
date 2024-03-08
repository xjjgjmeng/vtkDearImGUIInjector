#include <ImGuiCommon.h>

namespace
{
    class myCmd : public vtkCommand
    {
    public:
        static myCmd* New()
        {
            return new myCmd;
        }

        void Execute(vtkObject* caller, unsigned long, void*) override
        {
            int voi[6];
            pExtractVOI->GetVOI(voi);

            if (VTK_INT_MAX == voi[0] ||
                VTK_INT_MAX == voi[1] ||
                VTK_INT_MAX == voi[2] ||
                VTK_INT_MAX == voi[3] ||
                VTK_INT_MAX == voi[4] ||
                VTK_INT_MAX == voi[5])
            {
                return;
            }

            const double pt[3] = { voi[0], voi[2], voi[4] };
            double xyz[3];
            pImageData->TransformContinuousIndexToPhysicalPoint(pt, xyz);
            constexpr double color[3] = { 1.,0.,0. };
            const double pt2[3] = { voi[1], voi[3], voi[5] };
            double xyz2[3];
            pImageData->TransformContinuousIndexToPhysicalPoint(pt2, xyz2);
            constexpr double color2[3] = { 0.,1.,0. };
            for (const auto& [xyz, actor, color] : {std::tuple{xyz, this->pActor.Get(), color}, std::tuple{xyz2, this->pActor2.Get(), color2}})
            {
                vtkNew<vtkSphereSource> pSource;
                pSource->SetCenter(xyz);
                pSource->SetRadius(3.0);
                pSource->Update();

                vtkNew<vtkPolyDataMapper> pMapper;
                pMapper->SetInputConnection(pSource->GetOutputPort());

                actor->SetMapper(pMapper);
                actor->GetProperty()->SetColor(color[0], color[1], color[2]);
                actor->GetProperty()->SetPointSize(5);
                pRenderer->AddActor(actor);
            }

            {
                vtkNew<vtkOutlineSource> pSource;
                pSource->SetBounds(xyz[0], xyz2[0], xyz[1], xyz2[1], xyz[2], xyz2[2]);
                pSource->Update();

                vtkNew<vtkPolyDataMapper> pMapper;
                pMapper->SetInputData(pSource->GetOutput());

                pOutlineActor->SetMapper(pMapper);
                this->pRenderer->AddActor(pOutlineActor);
            }
        }

        vtkSmartPointer<vtkExtractVOI> pExtractVOI;
        vtkSmartPointer<vtkImageData> pImageData;
        vtkSmartPointer<vtkRenderer> pRenderer;
        vtkNew<vtkActor> pActor;
        vtkNew<vtkActor> pActor2;
        vtkNew<vtkActor> pOutlineActor;
    };
}

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();
    auto pImg = reader->GetOutput();

    vtkNew<vtkVolume> pVolume;
    vtkNew<vtkGPUVolumeRayCastMapper> pMapper;

    pMapper->SetInputData(pImg);
    pMapper->SetBlendModeToComposite();
    pVolume->SetMapper(pMapper);
    ::setupDefaultVolumeProperty(pVolume);
#if 1
    // 调节透明度让slice更清楚
    pVolume->GetProperty()->GetScalarOpacity()->RemoveAllPoints();
    pVolume->GetProperty()->GetScalarOpacity()->AddPoint(1000, 0.);
    pVolume->GetProperty()->GetScalarOpacity()->AddPoint(5000, 0.05);
#endif
    ren->AddVolume(pVolume);

    vtkNew<vtkExtractVOI> pExtractVOI;
    {
        vtkNew<myCmd> pCmd;
        pCmd->pExtractVOI = pExtractVOI;
        pCmd->pImageData = pImg;
        pCmd->pRenderer = ren;
        pExtractVOI->AddObserver(vtkCommand::ModifiedEvent, pCmd);
    }
    pExtractVOI->SetInputData(pImg);
    pExtractVOI->SetVOI(200, 600, 200, 600, 200, 300);
    pExtractVOI->Update();
    {
        vtkNew<vtkVolume> pVolume;
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;

        pMapper->SetInputConnection(pExtractVOI->GetOutputPort());
        pMapper->SetInputData(pExtractVOI->GetOutput());
        pMapper->SetBlendModeToComposite();
        pVolume->SetMapper(pMapper);
        ::setupDefaultVolumeProperty(pVolume);
        ren->AddVolume(pVolume);
    }

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            ImGuiNs::vtkObjSetup("ExtractVOI", pExtractVOI, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("OriginImg", pImg);
            ImGuiNs::vtkObjSetup("NewImg", pExtractVOI->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            //ImGuiNs::vtkObjSetup("Box", boxWidget, ImGuiTreeNodeFlags_DefaultOpen);
            //ImGuiNs::vtkObjSetup("Mapper", pMapper);
        };

    // Start rendering app
    ren->SetBackground(0., 0., 0.);
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
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();
    return 0;
}