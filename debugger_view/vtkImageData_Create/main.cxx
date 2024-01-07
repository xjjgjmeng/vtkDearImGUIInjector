#include <ImGuiCommon.h>

ImGuiNs::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    auto img = vtkSmartPointer<vtkImageData>::New();
    // 设置图像的维度、原点、间隔等信息
    img->SetDimensions(256, 256, 1);
    img->SetOrigin(0.0, 0.0, 0.0);
    img->SetSpacing(1.0, 1.0, 1.0);

    // 分配内存并写入数据
    img->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    unsigned char* data = static_cast<unsigned char*>(img->GetScalarPointer(0, 0, 0));

    for (int i = 0; i < 256; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            data[i * 256 + j] = static_cast<unsigned char>(i + j);
        }
    }
    auto actor = vtkSmartPointer<vtkImageActor>::New();
    actor->SetInputData(img);
    ren->AddActor(actor);

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            ImGuiNs::vtkObjSetup("ImageData", img, ImGuiTreeNodeFlags_DefaultOpen);
            ImGuiNs::vtkObjSetup("ImageActor", actor, ImGuiTreeNodeFlags_DefaultOpen);
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