#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    //vtkns::labelWorldZero(ren);

    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> lines;
    vtkNew<vtkPolyData> poly;

    points->InsertNextPoint(0, 0, 0);
    points->InsertNextPoint(0, 2, 0);
    points->InsertNextPoint(2, 4, 0);
    points->InsertNextPoint(4, 4, 0);
    lines->InsertNextCell(4);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(1);
    lines->InsertCellPoint(2);
    lines->InsertCellPoint(3);
    poly->SetPoints(points);
    poly->SetLines(lines);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(poly);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        //vtkns::vtkObjSetup("Data", data, ImGuiTreeNodeFlags_DefaultOpen);
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