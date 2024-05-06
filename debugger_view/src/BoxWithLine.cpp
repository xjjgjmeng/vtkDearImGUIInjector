#include <ImGuiCommon.h>

vtkns::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    auto renderer = ren.GetPointer();

    static auto lactor = vtkSmartPointer<vtkActor>::New();
    static auto ractor = vtkSmartPointer<vtkActor>::New();
    static vtkSmartPointer<vtkLineSource> lineSource;
    static vtkSmartPointer<vtkActor> lineActor;
    auto l = vtkSmartPointer<vtkCylinderSource>::New();
    auto lmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    lmapper->SetInputConnection(l->GetOutputPort());

    lactor->SetMapper(lmapper);
    lactor->GetProperty()->SetOpacity(0.3);
    lactor->SetPosition(1, 1, 1);
    renderer->AddActor(lactor);

    auto r = vtkSmartPointer<vtkCylinderSource>::New();
    auto rmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    rmapper->SetInputConnection(r->GetOutputPort());

    ractor->SetMapper(rmapper);
    ractor->GetProperty()->SetOpacity(0.3);
    renderer->AddActor(ractor);

    lineSource = vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(2.0, 2.0, 0.0);  // 起点坐标 
    lineSource->SetPoint2(3.0, 3.0, 0.0);  // 终点坐标
    lineSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper1->SetInputConnection(lineSource->GetOutputPort());

    lineActor = vtkSmartPointer<vtkActor>::New();
    lineActor->SetMapper(mapper1);
    renderer->AddActor(lineActor);

    renderer->SetBackground(0, 0, 0);
    renderer->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            auto reconnection = []
            {
                auto lCenter = lactor->GetCenter();
                auto rCenter = ractor->GetCenter();
                lineSource->SetPoint1(lCenter);
                lineSource->SetPoint2(rCenter);
            };

            double lPos[3];
            lactor->GetPosition(lPos);
            if (ImGui::DragScalarN("LPosition", ImGuiDataType_Double, lPos, static_cast<int>(std::size(lPos)), 0.01))
            {
                lactor->SetPosition(lPos);
                reconnection();
            }
            double rPos[3];
            ractor->GetPosition(rPos);
            if (ImGui::DragScalarN("RPosition", ImGuiDataType_Double, rPos, static_cast<int>(std::size(rPos)), 0.01))
            {
                ractor->SetPosition(rPos);
                reconnection();
            }

            static float lineColor[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
            if (ImGui::ColorEdit4("LineColor", lineColor))
            {
                lineActor->GetProperty()->SetColor(static_cast<double>(lineColor[0]), static_cast<double>(lineColor[1]), static_cast<double>(lineColor[2]));
                lineActor->GetProperty()->SetOpacity(static_cast<double>(lineColor[3]));
            }

            static bool bUseFXAA = ren->GetUseFXAA();
            if (ImGui::Checkbox("UseFXAA", &bUseFXAA))
            {
                ren->SetUseFXAA(bUseFXAA);
            }

            auto MultiSamples = renWin->GetMultiSamples();
            if (ImGui::DragInt("MultiSamples", &MultiSamples, 1, 0, 100))
            {
                renWin->SetMultiSamples(MultiSamples);
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