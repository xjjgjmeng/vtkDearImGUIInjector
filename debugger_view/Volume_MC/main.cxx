#include <ImGuiCommon.h>
#include <implot.h>

ImGuiNs::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkNamedColors> colors;

    vtkNew<vtkRenderWindow> renWin;
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(ImguiVtkNs::getDicomDir());
    reader->Update();

    vtkNew<vtkRenderer> ren;
    renWin->AddRenderer(ren);
    vtkNew<vtkVolume> pVolume;
    vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
    pMapper->SetInputData(reader->GetOutput());
    pMapper->SetBlendModeToComposite();
    pVolume->SetMapper(pMapper);
    ::setupDefaultVolumeProperty(pVolume);
    ren->AddVolume(pVolume);
    ren->SetViewport(0, 0, 0.333, 1);
    ren->ResetCamera();

    // vtkMarchingCubes
    vtkNew<vtkMarchingCubes> surface_mc;
    double isoValue_mc = 1000;
    surface_mc->SetInputData(reader->GetOutput());
    surface_mc->ComputeNormalsOn();
    surface_mc->SetValue(0, isoValue_mc);
    vtkNew<vtkPolyDataMapper> mapper_mc;
    mapper_mc->SetInputConnection(surface_mc->GetOutputPort());
    mapper_mc->ScalarVisibilityOff();
    vtkNew<vtkActor> actor_mc;
    actor_mc->SetMapper(mapper_mc);
    actor_mc->GetProperty()->SetColor(colors->GetColor3d("MistyRose").GetData());
    vtkNew<vtkRenderer> ren_mc;
    ren_mc->SetViewport(0.333, 0, 0.666, 1);
    ren_mc->AddActor(actor_mc);
    ren_mc->ResetCamera();
    renWin->AddRenderer(ren_mc);

    // vtkFlyingEdges3D
    vtkNew<vtkFlyingEdges3D> surface_fe;
    double isoValue_fe = 1000;
    surface_fe->SetInputData(reader->GetOutput());
    surface_fe->ComputeNormalsOn();
    surface_fe->SetValue(0, isoValue_fe);
    vtkNew<vtkPolyDataMapper> mapper_fe;
    mapper_fe->SetInputConnection(surface_fe->GetOutputPort());
    mapper_fe->ScalarVisibilityOff();
    vtkNew<vtkActor> actor_fe;
    actor_fe->SetMapper(mapper_fe);
    actor_fe->GetProperty()->SetColor(colors->GetColor3d("MistyRose").GetData());
    vtkNew<vtkRenderer> ren_fe;
    ren_fe->SetViewport(0.666, 0, 1, 1);
    ren_fe->AddActor(actor_fe);
    ren_fe->ResetCamera();
    renWin->AddRenderer(ren_fe);

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                logView.Draw();
                ImGui::TreePop();
            }
            ImGuiNs::vtkObjSetup("Volume", pVolume);
            ImGuiNs::vtkObjSetup("Mapper", pMapper);
            ImGuiNs::vtkObjSetup("MarchingCubes", surface_mc, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("MarchingCubesActor", actor_mc);
            ImGuiNs::vtkObjSetup("FlyingEdges3D", surface_fe, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("FlyingEdges3DActor", actor_fe);
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