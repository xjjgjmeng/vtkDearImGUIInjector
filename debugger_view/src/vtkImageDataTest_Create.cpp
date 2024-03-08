#include <ImGuiCommon.h>

//ImGuiNs::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    ImguiVtkNs::labelWorldZero(ren);

    vtkSmartPointer<vtkImageData> img;
    auto actor = vtkSmartPointer<vtkImageActor>::New();
    ren->AddActor(actor);

    double origin[3]{};
    double spacing[3] = {1.,1.,1.};
    int extentStart[3]{};

    auto generateImg = [&img, actor, &ren, &origin, &spacing, &extentStart] // arr??
    {
            int w = 100;
            int h = 100;

        img = vtkSmartPointer<vtkImageData>::New();
        // 设置图像的维度、原点、间隔等信息
#if 0
        img->SetDimensions(w, h, 1);
#else
        img->SetExtent(extentStart[0], extentStart[0] + w - 1, extentStart[1], extentStart[1] + h - 1, extentStart[2], extentStart[2]);
#endif
        // origin指的是extent (0,0,0)在世界坐标系中的位置
        img->SetOrigin(origin[0], origin[1], origin[2]);
        img->SetSpacing(spacing[0], spacing[1], spacing[2]);

        // 分配内存并写入数据
        img->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        //unsigned char* data = static_cast<unsigned char*>(img->GetScalarPointer(0, 0, 0));
        unsigned char* data = static_cast<unsigned char*>(img->GetScalarPointer(extentStart[0], extentStart[1], extentStart[2]));

        unsigned char grayval = 0;
        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                //data[i * w + j] = grayval++;//static_cast<unsigned char>(i + j);
                data[i * w + j] = static_cast<unsigned char>(i + j);
            }
        }

        actor->SetInputData(img);
    };
    generateImg();
    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::Button("generate"))
            {
                generateImg();
            }
            if (ImGui::DragScalarN("origin", ImGuiDataType_Double, origin, IM_ARRAYSIZE(origin), 0.1f))
            {
                generateImg();
            }
            if (ImGui::DragScalarN("spacing", ImGuiDataType_Double, spacing, IM_ARRAYSIZE(spacing), 0.001f))
            {
                generateImg();
            }
            if (ImGui::DragScalarN("extentStart", ImGuiDataType_S32, extentStart, IM_ARRAYSIZE(extentStart), 1))
            {
                generateImg();
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