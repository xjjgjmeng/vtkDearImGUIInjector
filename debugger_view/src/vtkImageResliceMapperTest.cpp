#include <ImGuiCommon.h>

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyStyle* New() { return new MyStyle; }

    void OnRightButtonDown() override
    {
        this->m_move = !this->m_move;
        __super::OnRightButtonDown();
    }

    void OnMouseMove() override
    {
        int currPos[2];
        this->Interactor->GetEventPosition(currPos);
        double displayPos[] = { currPos[0], currPos[1] };
        double worldPos[3];
        double worldOrient[9];
        const auto pRenderer = this->Interactor->FindPokedRenderer(currPos[0], currPos[1]);
        this->updataDisplayExtent();
        this->m_placer->ComputeWorldPosition(pRenderer, displayPos, worldPos, worldOrient);
        ::getLogView()->Add(fmt::format("EventPosition: {}", currPos));
        ::getLogView()->Add(fmt::format("worldPos: {::.2f}", worldPos));
        ::getLogView()->Add(fmt::format("worldOrient: {::.2f}\n", worldOrient));
        ::getLogView()->Add(fmt::format("ValidateWorldPosition: {}\n", this->m_placer->ValidateWorldPosition(worldPos)));
    
        if (m_move)
        {
            vtkNew<vtkSphereSource> pointSource;
            pointSource->SetCenter(worldPos);
            pointSource->SetRadius(5.0);
            pointSource->Update();

            vtkNew<vtkPolyDataMapper> mapper;
            mapper->SetInputConnection(pointSource->GetOutputPort());

            m_actor->SetMapper(mapper);
            if (this->m_placer->ValidateWorldPosition(worldPos))
            {
                m_actor->GetProperty()->SetColor(1, 0, 0);
            }
            else
            {
                m_actor->GetProperty()->SetColor(1, 1, 1);
            }
            m_actor->GetProperty()->SetPointSize(5);
            m_renderer->AddActor(m_actor);
        }

        __super::OnMouseMove();
    }

    vtkNew<vtkImageActorPointPlacer> m_placer;
    vtkSmartPointer<vtkRenderer> m_renderer;
    std::function<void()> updataDisplayExtent;
    vtkNew<vtkActor> m_actor;
    bool m_move = false;
};

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);
#if 0
    vtkNew<vtkXMLImageDataReader> reader;
    char* filename = "D:\\test_data\\Data\\straightCPR.vti";
    reader->SetFileName(filename);
    reader->Update();
#else
    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();
#endif
    auto pImageData = reader->GetOutput();

    vtkNew<vtkImageActor> pActor;
    vtkNew<vtkImageResliceMapper> pMapper;
    // 设置vtkImageResliceMapper。默认的是vtkImageSliceMapper
    {
        pMapper->SetInputData(pImageData);
        pMapper->SliceFacesCameraOff();
        pMapper->SliceAtFocalPointOff();
        {
            auto pPlane = pMapper->GetSlicePlane();
            pPlane->SetOrigin(pImageData->GetCenter());
            pPlane->SetNormal(1, 0, 0);
        }
        pMapper->UpdateInformation();
        pMapper->Modified();
        pActor->SetMapper(pMapper);
    }
    pActor->Update();
    ren->AddActor(pActor);
#define SHOW_Y 0
#define SHOW_Z 0
#if 1 == SHOW_Y
    // Y
    vtkNew<vtkImageActor> pYActor;
    vtkNew<vtkImageResliceMapper> pYMapper;
    {
        pYMapper->SetInputData(pImageData);
        pYMapper->SliceFacesCameraOff();
        pYMapper->SliceAtFocalPointOff();
        {
            auto pPlane = pYMapper->GetSlicePlane();
            pPlane->SetOrigin(pImageData->GetCenter());
            pPlane->SetNormal(0, 1, 0);
        }
        pYMapper->UpdateInformation();
        pYMapper->Modified();
        pYActor->SetMapper(pYMapper);
    }
    pYActor->Update();
    ren->AddActor(pYActor);
    pYActor->VisibilityOff();
#endif
#if 1 == SHOW_Z
    // Z
    vtkNew<vtkImageActor> pZActor;
    vtkNew<vtkImageResliceMapper> pZMapper;
    {
        pZMapper->SetInputData(pImageData);
        pZMapper->SliceFacesCameraOff();
        pZMapper->SliceAtFocalPointOff();
        {
            auto pPlane = pZMapper->GetSlicePlane();
            pPlane->SetOrigin(pImageData->GetCenter());
            pPlane->SetNormal(0, 0, 1);
        }
        pZMapper->UpdateInformation();
        pZMapper->Modified();
        pZActor->SetMapper(pZMapper);
    }
    pZActor->Update();
    ren->AddActor(pZActor);
    pZActor->VisibilityOff();
#endif

    ren->GetActiveCamera()->ParallelProjectionOn();
    ren->GetActiveCamera()->SetViewUp(0,1,0);
    ren->GetActiveCamera()->SetFocalPoint(pImageData->GetCenter());
    ren->GetActiveCamera()->SetPosition(pImageData->GetCenter()[0]+1, pImageData->GetCenter()[1], pImageData->GetCenter()[2]);
    ren->ResetCamera(pImageData->GetBounds());

    // 整体影像的轮廓
    {
        vtkNew<vtkImageDataOutlineFilter> pOutline;
        pOutline->SetInputData(pImageData);
        pOutline->Update();
        vtkNew<vtkPolyDataMapper> pMapper;
        pMapper->SetInputData(pOutline->GetOutput());
        vtkNew<vtkActor> pActor;
        pActor->SetMapper(pMapper);
        ren->AddActor(pActor);
    }
    // VR
    vtkNew<vtkVolume> pVolume;
    {
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
        pMapper->SetInputData(pImageData);
        pMapper->SetBlendModeToComposite();
        pVolume->SetMapper(pMapper);
        ::setupDefaultVolumeProperty(pVolume);
        pVolume->GetProperty()->GetScalarOpacity()->RemoveAllPoints();
        // 调节透明度让slice更清楚
        pVolume->GetProperty()->GetScalarOpacity()->AddPoint(1000, 0.);
        pVolume->GetProperty()->GetScalarOpacity()->AddPoint(3000, 0.5);
        ren->AddVolume(pVolume);
    }

    auto pStyle = vtkSmartPointer<MyStyle>::New();
    pStyle->m_placer->SetImageActor(pActor);
    pStyle->m_renderer = ren;
    pStyle->updataDisplayExtent = [&]
        {
            double imgIJK[3];
            pImageData->TransformPhysicalPointToContinuousIndex(pMapper->GetSlicePlane()->GetOrigin(), imgIJK);
            double v[6];
            pMapper->GetIndexBounds(v);
            v[0] = v[1] = imgIJK[0];
            pActor->SetDisplayExtent(std::round(v[0]), std::round(v[1]), std::round(v[2]), std::round(v[3]), std::round(v[4]), std::round(v[5]));
        };
    iren->SetInteractorStyle(pStyle);

    // ::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            ImGui::Text("Visibility: "); ImGui::SameLine();
            if (bool v = pActor->GetVisibility(); ImGui::Checkbox("XActor", &v)) pActor->SetVisibility(v); ImGui::SameLine();
#if 1 ==SHOW_Y
            if (bool v = pYActor->GetVisibility(); ImGui::Checkbox("YActor", &v)) pYActor->SetVisibility(v); ImGui::SameLine();
#endif
#if 1 == SHOW_Z
            if (bool v = pZActor->GetVisibility(); ImGui::Checkbox("ZActor", &v)) pZActor->SetVisibility(v); ImGui::SameLine();
#endif
            if (bool v = pVolume->GetVisibility(); ImGui::Checkbox("VRVisibility", &v)) pVolume->SetVisibility(v);

            //ImGui::Begin("666");
            ImGuiNs::vtkObjSetup("ImageData", reader->GetOutput());
            ImGuiNs::vtkObjSetup("ImageActor", pActor);
            //ImGui::End();
            ImGuiNs::vtkObjSetup("Mapper", pMapper, ImGuiTreeNodeFlags_DefaultOpen);
#if 1 == SHOW_Y
            ImGuiNs::vtkObjSetup("YMapper", pYMapper);
#endif
#if 1 == SHOW_Z
            ImGuiNs::vtkObjSetup("ZMapper", pZMapper);
#endif
        };

    // Start rendering app
    ren->SetBackground(0.2, 0.3, 0.4);
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}