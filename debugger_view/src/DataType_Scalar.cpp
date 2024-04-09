#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    //vtkns::labelWorldZero(ren);

    vtkNew<vtkPlaneSource> src;
    src->SetXResolution(5);
    src->SetYResolution(3);
    src->Update();
    auto poly = src->GetOutput();
    double rgb[][3] = { {255,0,0}, {0,255,0}, {0,0,255}, {255,255,0} };
    {
        vtkNew<vtkUnsignedCharArray> celldata;
        celldata->SetNumberOfComponents(3);
        celldata->SetNumberOfTuples(poly->GetNumberOfCells());
        for (auto i = 0; i < poly->GetNumberOfCells(); ++i)
        {
            celldata->InsertTuple(i, rgb[i % 4]);
        }
        poly->GetCellData()->SetScalars(celldata);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(poly);

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        ren->AddActor(actor);
    }

    {
        vtkNew<vtkPolyData> poly2;
        poly2->DeepCopy(poly);

        vtkNew<vtkUnsignedCharArray> pointData;
        pointData->SetNumberOfComponents(3);
        pointData->SetNumberOfTuples(poly2->GetNumberOfPoints());
        for (auto i = 0; i < poly2->GetNumberOfPoints(); ++i)
        {
            pointData->InsertTuple(i, rgb[i % 4]);
        }
        poly2->GetPointData()->SetScalars(pointData);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(poly2);

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->AddPosition(1.3, 0, 0);
        ren->AddActor(actor);
    }

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        //vtkns::vtkObjSetup("Data", data, ImGuiTreeNodeFlags_DefaultOpen);
        //vtkns::vtkObjSetup("Actor", actor);
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