#include <ImGuiCommon.h>

vtkns::LogView logView;

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyStyle* New();
    vtkTypeMacro(MyStyle, vtkInteractorStyleTrackballCamera);

    void OnLeftButtonDown() override
    {
        double worldPt[4];
        vtkInteractorObserver::ComputeDisplayToWorld(
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer(),
            this->Interactor->GetEventPosition()[0],
            this->Interactor->GetEventPosition()[1],
            0,
            worldPt);
        logView.Add(fmt::format("worldPt0: {}", worldPt));
        this->m_points->InsertNextPoint(worldPt);
        vtkSplineRepresentation::SafeDownCast(this->m_spline->GetRepresentation())->InitializeHandles(this->m_points);
    }

    vtkSplineWidget2* m_spline = nullptr;
    vtkNew<vtkPoints> m_points;
};
vtkStandardNewMacro(MyStyle);

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkSplineWidget2> splineWidget;
    {
        splineWidget->SetInteractor(iren);
        renWin->Render();
        splineWidget->On();
        ren->ResetCamera(); // 否则界面点击无效
    }
    auto pStyle = vtkSmartPointer<MyStyle>::New();
    pStyle->m_spline = splineWidget;
    iren->SetInteractorStyle(pStyle);

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        if (ImGui::TreeNode("Log"))
        {
            logView.Draw();
            ImGui::TreePop();
        }
        vtkns::vtkObjSetup("SplineWidget2", splineWidget, ImGuiTreeNodeFlags_DefaultOpen);
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