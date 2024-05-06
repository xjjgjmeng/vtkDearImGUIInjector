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

    vtkNew<vtkPlaneSource> src;
    {
        vtkNew<vtkCallbackCommand> cc;
        {
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                {
                    auto p1 = vtkPlaneSource::SafeDownCast(caller)->GetPoint1();
                    auto p2 = vtkPlaneSource::SafeDownCast(caller)->GetPoint2();
                    auto o = vtkPlaneSource::SafeDownCast(caller)->GetOrigin();
                    auto c = vtkPlaneSource::SafeDownCast(caller)->GetCenter();
                    vtkns::makePoints({ {p1[0], p1[1], p1[2]}, {p2[0], p2[1], p2[2]} , {o[0], o[1], o[2]}, {c[0], c[1], c[2]} }, reinterpret_cast<vtkActor*>(clientdata));
                };
            cc->SetCallback(f);
            vtkNew<vtkActor> actor;
            actor->GetProperty()->SetColor(1, 0, 0);
            actor->GetProperty()->SetPointSize(12);
            actor->GetProperty()->SetRenderPointsAsSpheres(1); // ??
            ren->AddActor(actor);
            cc->SetClientData(actor);
        }
        src->AddObserver(vtkCommand::ModifiedEvent, cc);
    }
    {
#if 0 // 按w键查看
        src->SetXResolution(3);
        src->SetYResolution(4);
#else
        src->SetXResolution(50);
        src->SetYResolution(50);
        src->SetOrigin(0, 0, 0);
        src->SetPoint1(30, 0, 0);
        src->SetPoint2(0, 30, 0);
        src->SetCenter(30, 30, 0);
        src->SetNormal(1, 1, 1);
#endif
    }
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(src->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("src", src, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("actor", actor);
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