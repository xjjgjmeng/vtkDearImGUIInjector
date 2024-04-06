#include <ImGuiCommon.h>



int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    std::array<std::array<double, 3>, 8> pts = {
        {
            {0,0,0},
            {1,0,0},
            {1,1,0},
            {0,1,0},
            {0,0,1},
            {1,0,1},
            {1,1,1},
            {0,1,1},
        }
    };

    std::array<std::array<vtkIdType, 4>, 6> ordering = {
        {
            {0,1,2,3},
            {4,5,6,7},
            {0,1,5,4},
            {1,2,6,5},
            {2,3,7,6},
            {3,0,4,7},
        }
    };

    vtkNew<vtkPolyData> cube;
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> polys;
    vtkNew<vtkFloatArray> scalars;
    for (std::size_t i = 0; i < pts.size(); ++i)
    {
        points->InsertPoint(i, pts[i].data());
        scalars->InsertTuple1(i, i);
    }

    for (auto&& i : ordering)
    {
        polys->InsertNextCell(vtkIdType(i.size()), i.data());
    }
    cube->SetPoints(points);
    cube->GetPointData()->SetScalars(scalars);
    cube->SetPolys(polys);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(cube);
    mapper->SetScalarRange(cube->GetScalarRange());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    //actor->GetProperty()->EdgeVisibilityOn();
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}