#include <ImGuiCommon.h>

ImGuiNs::LogView logView;

class MyStyle : public vtkInteractorStyleImage
{
public:
    static MyStyle* New() { return new MyStyle; }

    void OnMouseMove() override
    {
        int currPos[2];
        this->GetInteractor()->GetEventPosition(currPos);
        double displayPos[] = { currPos[0], currPos[1] };
        double worldPos[3];
        double worldOrient[9];
        const auto pRenderer = this->GetInteractor()->FindPokedRenderer(currPos[0], currPos[1]);
        this->m_placer->ComputeWorldPosition(pRenderer, displayPos, worldPos, worldOrient);
        ::logView.Add(fmt::format("EventPosition: {}", currPos));
        ::logView.Add(fmt::format("worldPos: {}", worldPos));
        ::logView.Add(fmt::format("worldOrient: {}\n", worldOrient));
    }

    vtkNew<vtkImageActorPointPlacer> m_placer;
};

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindowInteractor> iren;
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->SetMultiSamples(8);
    renderWindow->AddRenderer(renderer);
    iren->SetRenderWindow(renderWindow);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();
    auto pImageData = reader->GetOutput();

    vtkNew<vtkImageSliceMapper> pXMapper;
    pXMapper->SetInputConnection(reader->GetOutputPort());
    pXMapper->SetOrientationToX();
    vtkNew<vtkImageActor> xActor;
    xActor->SetMapper(pXMapper);
    renderer->AddActor(xActor);

    vtkNew<vtkImageSliceMapper> pYMapper;
    pYMapper->SetInputConnection(reader->GetOutputPort());
    pYMapper->SetOrientationToY();
    vtkNew<vtkImageActor> yActor;
    yActor->SetMapper(pYMapper);
    renderer->AddActor(yActor);

    vtkNew<vtkImageSliceMapper> pZMapper;
    pZMapper->SetInputConnection(reader->GetOutputPort());
    pZMapper->SetOrientationToZ();
    vtkNew<vtkImageActor> zActor;
    zActor->SetMapper(pZMapper);
    renderer->AddActor(zActor);

    // 整体影像的轮廓
    {
        vtkNew<vtkImageDataOutlineFilter> pOutline;
        pOutline->SetInputData(pImageData);
        pOutline->Update();
        vtkNew<vtkPolyDataMapper> pMapper;
        pMapper->SetInputData(pOutline->GetOutput());
        vtkNew<vtkActor> pActor;
        pActor->SetMapper(pMapper);
        renderer->AddActor(pActor);
    }
    // VR
    {
        vtkNew<vtkVolume> pVolume;
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
        pMapper->SetInputData(pImageData);
        pMapper->SetBlendModeToComposite();
        pVolume->SetMapper(pMapper);
        ::setupDefaultVolumeProperty(pVolume);
        pVolume->GetProperty()->GetScalarOpacity()->RemoveAllPoints();
        // 调节透明度让slice更清楚
        pVolume->GetProperty()->GetScalarOpacity()->AddPoint(1000, 0.);
        pVolume->GetProperty()->GetScalarOpacity()->AddPoint(3000, 0.5);
        renderer->AddVolume(pVolume);
    }

    renderer->ResetCamera();

    ::pWindow = renderWindow;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            ImGuiNs::vtkObjSetup("vtkImageData", reader->GetOutput());
            ImGuiNs::vtkObjSetup("XImageActor", xActor);
            ImGuiNs::vtkObjSetup("XMapper", pXMapper, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("YImageActor", yActor);
            ImGuiNs::vtkObjSetup("YMapper", pYMapper, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("ZImageActor", zActor);
            ImGuiNs::vtkObjSetup("ZMapper", pZMapper, ImGuiTreeNodeFlags_DefaultOpen);
            if (ImGui::Button("recalcX"))
            {
                int dims[3];
                reader->GetOutput()->GetDimensions(dims);
                xActor->SetDisplayExtent(pXMapper->GetSliceNumber(), pXMapper->GetSliceNumber(), 0, dims[1], 0, dims[2]);
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
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}