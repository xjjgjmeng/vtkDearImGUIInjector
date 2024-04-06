#include <ImGuiCommon.h>

vtkns::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkNamedColors> colors;

    // Sphere
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(
        colors->GetColor3d("DarkOliveGreen").GetData());
    ren->AddActor(actor);

    // Create the widget and its representation

    vtkNew<vtkCaptionRepresentation> captionRepresentation;
    captionRepresentation->GetCaptionActor2D()->SetCaption("Test caption");
    captionRepresentation->GetCaptionActor2D()
        ->GetTextActor()
        ->GetTextProperty()
        ->SetFontSize(100);

    double pos[3] = { .5, 0, 0 };
    captionRepresentation->SetAnchorPosition(pos);

    vtkNew<vtkCaptionWidget> captionWidget;
    captionWidget->SetInteractor(renWin->GetInteractor());
    captionWidget->SetRepresentation(captionRepresentation);
    //ren->GetActiveCamera(); // 不写这句报错：WorldToView: no active camera, cannot compute world to view, returning 0,0,0
    ren->GetActiveCamera()->Azimuth(90);
    ren->ResetCamera();
    renWin->Render(); // ??
    captionWidget->On();
    ren->SetBackground(colors->GetColor3d("Blue").GetData());

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            bool captionOn = captionWidget->GetEnabled();
            if (ImGui::Checkbox("On", &captionOn))
            {
                captionWidget->SetEnabled(captionOn);
            }
            ImGui::SameLine();
            bool bLeader = captionRepresentation->GetCaptionActor2D()->GetLeader();
            if (ImGui::Checkbox("Leader", &bLeader))
            {
                captionRepresentation->GetCaptionActor2D()->SetLeader(bLeader);
            }
            ImGui::SameLine();
            bool bBorder = captionRepresentation->GetCaptionActor2D()->GetBorder();
            if (ImGui::Checkbox("Border", &bBorder))
            {
                captionRepresentation->GetCaptionActor2D()->SetBorder(bBorder);
            }

            double pos[3];
            captionRepresentation->GetAnchorPosition(pos);
            if (ImGui::DragScalarN("AnchorPosition", ImGuiDataType_Double, pos, 3, 0.001))
            {
                captionRepresentation->SetAnchorPosition(pos);
            }

            int fontSize = captionRepresentation->GetCaptionActor2D()
                ->GetTextActor()
                ->GetTextProperty()
                ->GetFontSize();
            if (ImGui::SliderInt("FontSize", &fontSize, 1, 1000))
            {
                captionRepresentation->GetCaptionActor2D()
                    ->GetTextActor()
                    ->GetTextProperty()
                    ->SetFontSize(fontSize);
            }
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}