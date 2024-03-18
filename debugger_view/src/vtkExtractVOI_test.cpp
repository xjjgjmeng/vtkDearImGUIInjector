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

            // 将最新提取出来的volume的轮廓线显示出来
            {
                vtkNew<vtkOutlineSource> pSource;
                // 参数是世界坐标xyz对:xmin,xmax,ymin,ymax,zmin,zmax
                pSource->SetBounds(xyz[0], xyz2[0], xyz[1], xyz2[1], xyz[2], xyz2[2]);
                pSource->Update();

                vtkNew<vtkPolyDataMapper> pMapper;
                pMapper->SetInputData(pSource->GetOutput());

                pOutlineActor->SetMapper(pMapper);
                this->pRenderer->AddActor(pOutlineActor);
            }

            {
                // 显示出来的单层slice在世界坐标和三维数据是对齐的
                // 通过拖拽vtkImageActor的Mapper的SliceNumber可以看slice在voi的空间范围内移动
                this->pImgActor->SetInputData(this->pExtractVOI->GetOutput());
                this->pRenderer->AddActor(this->pImgActor);
            }
        }

        vtkSmartPointer<vtkExtractVOI> pExtractVOI;
        vtkSmartPointer<vtkImageData> pImageData;
        vtkSmartPointer<vtkRenderer> pRenderer;
        vtkNew<vtkActor> pActor;
        vtkNew<vtkActor> pActor2;
        vtkNew<vtkActor> pOutlineActor;
        vtkSmartPointer<vtkImageActor> pImgActor;
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

    ImguiVtkNs::genImgOutline(ren, pImg);

#if 1
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
#endif

    vtkNew<vtkExtractVOI> pExtractVOI;
    vtkNew<vtkImageActor> pImgActor;
    {
        vtkNew<myCmd> pCmd;
        pCmd->pExtractVOI = pExtractVOI;
        pCmd->pImageData = pImg;
        pCmd->pRenderer = ren;
        pCmd->pImgActor = pImgActor;
        pExtractVOI->AddObserver(vtkCommand::ModifiedEvent, pCmd);
    }
    // Extract
    {
        pExtractVOI->SetInputData(pImg);
        int ijk[6];
        pImg->GetExtent(ijk);
        // 只提取中间300X300X300的volume
        for (auto minval : { ijk +0, ijk + 2, ijk + 4 })
        {
            while ((*(minval + 1) - *minval) > 300)
            {
                --(*(minval + 1));
                ++(*minval);
            }
        }
        // 参数是索引对:imin,imax,jmin,jmax,kmin,kmax
        // 如果其中有一个索引对是一样的，比如imin等于imax，那么输出的vtkImageData的DataDimension将是2
        // 如果imin>imax，那么输出的vtkImageData的DataDimension将是0
        pExtractVOI->SetVOI(ijk[0], ijk[1], ijk[2], ijk[3], ijk[4], ijk[5]);
        pExtractVOI->Update();
    }
    // 将提取的volume渲染出来
    {
        vtkNew<vtkVolume> pVolume;
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;

        pMapper->SetInputConnection(pExtractVOI->GetOutputPort());
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
            ImGuiNs::vtkObjSetup("ImgActor", pImgActor);
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