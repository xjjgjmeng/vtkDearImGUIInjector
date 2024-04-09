#include <ImGuiCommon.h>

//ImGuiNs::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkns::labelWorldZero(ren);

    vtkSmartPointer<vtkImageData> img;
    auto actor = vtkSmartPointer<vtkImageActor>::New();
    ren->AddActor(actor);

    //vtkNew<vtkActor> polydataActor;
    //vtkSmartPointer<vtkImageToPolyDataFilter> imgPolyFilter = vtkSmartPointer<vtkImageToPolyDataFilter>::New();
    //{
    //    vtkNew<vtkRenderer> ren;
    //    ren->SetBackground(1, 1, 1);
    //    ren->AddActor(polydataActor);
    //    ren->SetViewport(0.8, 0.5, 1, 1);
    //    renWin->AddRenderer(ren);
    //}

    vtkNew<vtkActor> dataSetActor;
    dataSetActor->AddPosition(3, 3, 3);
    dataSetActor->GetProperty()->SetRepresentationToWireframe();
    dataSetActor->GetProperty()->SetColor(1, 1, 0);
    ren->AddActor(dataSetActor);

    double origin[3]{};
    double spacing[3] = { 1.,1.,1. };
    int extentStart[3]{};
    int w = 100;
    int h = 100;

    auto generateImg = [&] // arr??
    {
        img = vtkSmartPointer<vtkImageData>::New();
        // 设置图像的维度、原点、间隔等信息
#if 0
        img->SetDimensions(w, h, 1);
#else
        img->SetExtent(extentStart[0], extentStart[0] + w - 1, extentStart[1], extentStart[1] + h - 1, extentStart[2], extentStart[2]);
#endif
        // origin指的是extent (0,0,0)在世界坐标系中的位置
        img->SetOrigin(origin);
        img->SetSpacing(spacing);

        // 分配内存并写入数据
        img->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
#if 0 // 会崩溃
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
#endif
        actor->SetInputData(img);

        {
            vtkNew<vtkDataSetMapper> mapper;
#if 1
            vtkNew<vtkImageData> data;
            data->SetOrigin(img->GetOrigin());
            data->SetSpacing(img->GetSpacing());
            data->SetExtent(img->GetExtent());
            mapper->SetInputData(data);
#else // 这样写设置Color不生效
            mapper->SetInputData(img);
#endif
            dataSetActor->SetMapper(mapper);
        }
#if 0
        {
            //vtkNew<vtkImageToPolyDataFilter> filter;
            //imgPolyFilter = filter.GetPointer();
            imgPolyFilter->SetColorModeToLUT();
            {
                vtkNew<vtkScalarsToColors> lut;
                lut->SetRange(0, 255);
                imgPolyFilter->SetLookupTable(lut);
            }
            imgPolyFilter->SetInputData(img);
            vtkNew<vtkPolyDataMapper> mapper;
            mapper->SetInputConnection(imgPolyFilter->GetOutputPort());
            polydataActor->SetMapper(mapper);
            polydataActor->GetProperty()->SetColor(1, 0, 0);
        }
#endif
    };
    generateImg();
    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
    {
        if (ImGui::TreeNodeEx(u8"生成参数", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::DragInt("Width", &w)) generateImg();
            if (ImGui::DragInt("height", &h)) generateImg();
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

            ImGui::TreePop();
        }
        vtkns::vtkObjSetup("ImageData", img, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("ImageActor", actor);
        vtkns::vtkObjSetup("DataSetActor", dataSetActor);
        //vtkns::vtkObjSetup("polydataActor", polydataActor, ImGuiTreeNodeFlags_DefaultOpen);
        //vtkns::vtkObjSetup("filter", imgPolyFilter, ImGuiTreeNodeFlags_DefaultOpen);
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