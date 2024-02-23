#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkImageViewer2> pViewer;
    vtkNew<vtkRenderWindowInteractor> iren;
    pViewer->SetupInteractor(iren);
    auto ren = pViewer->GetRenderer();
    auto renWin = pViewer->GetRenderWindow();
    iren->SetInteractorStyle(vtkNew<vtkInteractorStyleTrackballCamera>{});
    pViewer->GetRenderer()->GetActiveCamera()->SetParallelProjection(false);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();

    {
        vtkNew<vtkVolume> pVolume;
        vtkNew<vtkGPUVolumeRayCastMapper> pMapper;

        pMapper->SetInputData(reader->GetOutput());
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

        {
            vtkNew<vtkVolumeOutlineSource> pVOS;
            vtkNew<vtkActor> vosActor;
            {
                pVOS->SetVolumeMapper(pMapper);
                vtkNew<vtkPolyDataMapper> mapper;
                mapper->SetInputConnection(pVOS->GetOutputPort());
                vosActor->SetMapper(mapper);
                ren->AddActor(vosActor);
            }
        }
    }

    vtkNew<vtkImageSlab> slab;
    slab->SetMultiSliceOutput(1);
    slab->SetInputData(reader->GetOutput());
    slab->SetOrientationToZ();
    slab->SetOperationToMean();
    slab->SetTrapezoidIntegration(1);
    slab->SetSliceRange(0,3);
    slab->SetEnableSMP(1);
    slab->Update();

    vtkNew<vtkImageChangeInformation> changer;
    changer->SetInputConnection(slab->GetOutputPort());
    changer->SetOutputExtentStart(0, 0, 0);
    changer->SetOutputOrigin(0, 0, 0);
    changer->SetOutputSpacing(reader->GetOutput()->GetSpacing());
    changer->Update();

    pViewer->SetInputConnection(changer->GetOutputPort());

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            ImGui::Begin("OLD");
            ImGuiNs::vtkObjSetup("image_I", reader->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();

            ImGui::Begin("NEW");
            ImGuiNs::vtkObjSetup("image_O", changer->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
            
            ImGuiNs::vtkObjSetup("Slab", slab, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("Viewer2", pViewer, ImGuiTreeNodeFlags_DefaultOpen);
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();
    return 0;
}