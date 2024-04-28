#include <ImGuiCommon.h>

int main()
{
    SETUP_WINDOW
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    vtkNew<vtkAppendPolyData> polydataAppender;
    // 生成polydata
    {
        {
            vtkNew<vtkConeSource> src;
            src->SetCenter(img->GetCenter());
            src->SetHeight(100);
            src->SetRadius(50);
            src->SetDirection(0, 0, 1);
            src->Update();
            polydataAppender->AddInputData(src->GetOutput());
        }
        {
            vtkNew<vtkSphereSource> src;
            src->SetCenter(img->GetBounds()[0], img->GetBounds()[2], img->GetCenter()[2]);
            src->SetRadius(23);
            src->Update();
            polydataAppender->AddInputData(src->GetOutput());
        }
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(polydataAppender->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        ren->AddViewProp(actor);
    }

    auto reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputData(img);
    reslice->SetOutputDimensionality(2);
    {
        vtkNew<vtkImageActor> resliceImgActor;
        resliceImgActor->GetProperty()->SetColorWindow(6000);
        resliceImgActor->GetProperty()->SetColorLevel(2000);
        resliceImgActor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
        ren->AddActor(resliceImgActor);
    }

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false);
    vtkns::genVR(ren, img, false, true);

    {
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        reslice->SetResliceAxesDirectionCosines(x, y, z);
        {
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
            {
                vtkns::mat::genAxes(reinterpret_cast<vtkRenderer*>(clientdata), vtkMatrix4x4::SafeDownCast(caller));
            };
            vtkNew<vtkCallbackCommand> pCC;
            pCC->SetCallback(f);
            pCC->SetClientData(ren);
            reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, pCC);
        }
        reslice->SetResliceAxesOrigin(img->GetCenter());
        reslice->SetInterpolationModeToLinear();
    }
    //{
        vtkNew<vtkImageThreshold> thresholdFilter;
        thresholdFilter->SetInputData(img);
        thresholdFilter->ThresholdByUpper(1);
        thresholdFilter->SetInValue(1);
        thresholdFilter->SetOutValue(1);
        thresholdFilter->Update();

        vtkNew<vtkPolyDataToImageStencil> p2iStencil;
        p2iStencil->SetInputConnection(polydataAppender->GetOutputPort());
        p2iStencil->SetOutputOrigin(img->GetOrigin());
        p2iStencil->SetOutputSpacing(img->GetSpacing());
        p2iStencil->SetOutputWholeExtent(img->GetExtent());
        p2iStencil->Update();
        vtkNew<vtkImageStencil> imgStencil;
        imgStencil->SetInputConnection(thresholdFilter->GetOutputPort());
        imgStencil->SetStencilConnection(p2iStencil->GetOutputPort());
        imgStencil->ReverseStencilOff();
        imgStencil->SetBackgroundValue(0);

        vtkNew<vtkImageReslice> maskReslice;
        maskReslice->SetInputConnection(imgStencil->GetOutputPort());
        maskReslice->SetOutputDimensionality(2);
        maskReslice->SetResliceAxes(reslice->GetResliceAxes()); // 使用同一个矩阵
        maskReslice->InterpolateOff();
        maskReslice->AutoCropOutputOn();
        maskReslice->Update();

        vtkNew<vtkImageMapToColors> maskColors;
        {
            vtkNew<vtkLookupTable> lut;
            //lut->SetNumberOfColors(2);
            lut->SetNumberOfTableValues(2);
            lut->SetTableValue(0, 1, 0, 0, 0);
            lut->SetTableValue(1, 1, 0, 0, 1);
            lut->Build();
            maskColors->SetLookupTable(lut);
        }
        maskColors->SetInputConnection(maskReslice->GetOutputPort());
        vtkNew<vtkImageActor> maskActor;
        maskActor->GetMapper()->SetInputConnection(maskColors->GetOutputPort());
        maskActor->SetPickable(false);
        ren->AddViewProp(maskActor);
    //}

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("Reslice", reslice, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("maskColors", maskColors, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("imgStencil", imgStencil, ImGuiTreeNodeFlags_DefaultOpen);
    };

    // Start rendering app
    rw->Render();

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(rwi);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
// 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, rw->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    vtkInteractorStyleSwitch::SafeDownCast(rwi->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    rwi->EnableRenderOff();
    rwi->Start();
}