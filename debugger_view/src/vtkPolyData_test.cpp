#include <ImGuiCommon.h>



int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);


    vtkNew<vtkPoints> points;
    points->InsertPoint(0, 0, 0, 0);
    points->InsertPoint(1, 0, 1, 0);
    points->InsertPoint(2, 1, 0, 0);
    points->InsertPoint(3, 1, 1, 0);
    points->InsertPoint(4, 2, 0, 0);
    points->InsertPoint(5, 2, 1, 0);
    points->InsertPoint(6, 3, 0, 0);
    points->InsertPoint(7, 3, 1, 0);

    vtkNew<vtkCellArray> strips;
    strips->InsertNextCell(8);
    strips->InsertCellPoint(0);
    strips->InsertCellPoint(1);
    strips->InsertCellPoint(2);
    strips->InsertCellPoint(3);
    strips->InsertCellPoint(4);
    strips->InsertCellPoint(5);
    strips->InsertCellPoint(6);
    strips->InsertCellPoint(7);

    vtkNew<vtkPolyData> poly;
    poly->SetPoints(points);
    //poly->SetStrips(strips);
    //poly->SetLines(strips);
    poly->SetVerts(strips);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(poly);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->EdgeVisibilityOn();
    ren->AddActor(actor);

    // ::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            //ImGuiNs::vtkObjSetup("OriginImage", reader->GetOutput());
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