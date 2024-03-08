#include <ImGuiCommon.h>

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    ImguiVtkNs::labelWorldZero(ren);

    auto pCaptionActor2D = vtkSmartPointer<vtkCaptionActor2D>::New();
    pCaptionActor2D->SetCaption("123\n456\nhello");
    pCaptionActor2D->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    pCaptionActor2D->GetActualPositionCoordinate()->SetCoordinateSystemToWorld();
    pCaptionActor2D->SetAttachmentPoint(0, 0, 0);
    pCaptionActor2D->GetTextActor()->SetTextScaleModeToNone();//不设置这句字体大小不起作用   默认SetTextScaleModeToProp
    pCaptionActor2D->GetCaptionTextProperty()->SetFontSize(15);
    pCaptionActor2D->GetCaptionTextProperty()->SetColor(1, 1, 1);
    pCaptionActor2D->GetCaptionTextProperty()->SetBold(1);
    pCaptionActor2D->GetCaptionTextProperty()->SetItalic(1);
    pCaptionActor2D->GetCaptionTextProperty()->SetShadow(1);
    pCaptionActor2D->GetCaptionTextProperty()->SetFontFamilyToArial();
    ren->AddActor(pCaptionActor2D);

    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            ImGuiNs::vtkObjSetup("CaptionActor2D", pCaptionActor2D, ImGuiTreeNodeFlags_DefaultOpen);
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
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}