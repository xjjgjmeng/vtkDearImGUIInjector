#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkns::labelWorldZero(ren);

    vtkNew<vtkUnstructuredGrid> data;
    vtkNew<vtkPoints> points;
    points->SetNumberOfPoints(8);
    points->InsertPoint(0, 0, 0, 0);
    points->InsertPoint(1, 1, 0, 0);
    points->InsertPoint(2, 1, 1, 0);
    points->InsertPoint(3, 0, 1, 0);
    points->InsertPoint(4, 0, 0, 1);
    points->InsertPoint(5, 1, 0, 1);
    points->InsertPoint(6, 1, 1, 1);
    points->InsertPoint(7, 0, 1, 1);
    vtkNew<vtkHexahedron> cell;
    //vtkNew<vtkVoxel> cell;
    cell->GetPointIds()->SetId(0, 0);
    cell->GetPointIds()->SetId(1, 1);
    cell->GetPointIds()->SetId(2, 2);
    cell->GetPointIds()->SetId(3, 3);
    cell->GetPointIds()->SetId(4, 4);
    cell->GetPointIds()->SetId(5, 5);
    cell->GetPointIds()->SetId(6, 6);
    cell->GetPointIds()->SetId(7, 7);
    data->SetPoints(points);
    data->InsertNextCell(cell->GetCellType(), cell->GetPointIds());

    vtkNew<vtkDataSetMapper> mapper;
    mapper->SetInputData(data);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    {
        vtkNew<vtkPoints> points;
        points->SetNumberOfPoints(4);
        points->InsertPoint(0,0,0,0);
        points->InsertPoint(1,1,0,0);
        points->InsertPoint(2,0.5,1,0);
        points->InsertPoint(3,0.5,0.5,1);
        vtkNew<vtkTetra> cell;
        cell->GetPointIds()->SetId(0, 0);
        cell->GetPointIds()->SetId(1, 1);
        cell->GetPointIds()->SetId(2, 2);
        cell->GetPointIds()->SetId(3, 3);
        vtkNew<vtkUnstructuredGrid> data;
        data->SetPoints(points);
        data->InsertNextCell(cell->GetCellType(), cell->GetPointIds());

        vtkNew<vtkDataSetMapper> mapper;
        mapper->SetInputData(data);

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->AddPosition(3, 0, 0);
        ren->AddActor(actor);
    }

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