#include <ImGuiCommon.h>
#include <PolyDataHelper.h>


int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkns::labelWorldZero(ren);

    vtkNs::Pt_t p0{0,0,0};
    vtkNs::Pt_t p1{10,10,10};

    vtkNew<vtkActor> actor;
    vtkNs::makeLines({ p0, p1 }, actor);
    ren->AddActor(actor);

    auto pd = vtkNs::makeLines({ p0, p1 });
    vtkNew<vtkTransform> transform;
    transform->Scale(2, 3, 4);
    transform->Translate(10, 0, 0);
    vtkNew<vtkTransformPolyDataFilter> filter;
    filter->SetInputData(pd);
    filter->SetTransform(transform);
    filter->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(filter->GetOutput());
    vtkNew<vtkActor> transformActor;
    transformActor->GetProperty()->SetColor(1, 1, 0);
    transformActor->SetMapper(mapper);
    ren->AddActor(transformActor);

    // ::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("filter", filter, ImGuiTreeNodeFlags_DefaultOpen);
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