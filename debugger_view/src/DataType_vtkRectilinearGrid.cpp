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
    vtkNew<vtkRectilinearGrid> data;
    data->SetExtent(0, gridSize - 1, 0, gridSize - 1, 0, gridSize - 1);
    vtkNew<vtkDoubleArray> xCoords;
    xCoords->SetNumberOfComponents(1);
    vtkNew<vtkDoubleArray> yCoords;
    yCoords->SetNumberOfComponents(1);
    vtkNew<vtkDoubleArray> zCoords;
    zCoords->SetNumberOfComponents(1);
    for (auto i = 0; i < gridSize; ++i)
    {
        if (0 == i)
        {
            xCoords->InsertNextValue(0);
            yCoords->InsertNextValue(0);
            zCoords->InsertNextValue(0);
            continue;
        }
        double oldX = xCoords->GetValue(i - 1);
        double oldY = yCoords->GetValue(i - 1);
        double oldZ = zCoords->GetValue(i - 1);
        xCoords->InsertNextValue(oldX + i * i);
        yCoords->InsertNextValue(oldY + i * i);
        zCoords->InsertNextValue(oldZ + i * i);
    }
    data->SetXCoordinates(xCoords);
    data->SetYCoordinates(yCoords);
    data->SetZCoordinates(zCoords);

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