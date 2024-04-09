#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkns::labelWorldZero(ren);

    constexpr auto gridSize = 5;
    vtkNew<vtkStructuredGrid> data;
    data->SetExtent(0, gridSize - 1, 0, gridSize - 1, 0, gridSize - 1);
    vtkNew<vtkPoints> points;
    for (auto i = 0; i < gridSize; ++i)
    {
        for (auto j = 0; j < gridSize; ++j)
        {
            for (auto k = 0; k < gridSize; ++k)
            {
                points->InsertNextPoint(i, j + i * .5, k);
            }
        }
    }
    data->SetPoints(points);

    vtkNew<vtkDataSetMapper> mapper;
    mapper->SetInputData(data);

    vtkNew<vtkActor> actor;
    actor->GetProperty()->SetRepresentationToWireframe();
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("Data", data, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("Actor", actor);
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
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
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