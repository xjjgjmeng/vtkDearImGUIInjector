#include <ImGuiCommon.h>
#include <PolyDataHelper.h>

vtkSmartPointer<vtkImageReslice> reslice;
vtkSmartPointer<vtkImageChangeInformation> changer;
double spacing[3];
vtkNew<vtkActor> vrPtChangerActor;
vtkSmartPointer<vtkImageData> img;

int main()
{
    SETUP_WINDOW
    img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    int extent[6];
    double origin[3];
    img->GetExtent(extent);
    img->GetSpacing(::spacing);
    img->GetOrigin(origin);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false);// ->GetProperty()->SetColor(1., 1., 0.);
    //vtkns::genVR(ren, img, false, true);
    if (3 == ::reslice->GetOutputDimensionality())
    {
        // 将切割出来的体数据渲染出来
        vtkns::genVR(ren, reslice->GetOutput(), false, true);
    }

    {
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        ::reslice->SetResliceAxesDirectionCosines(x, y, z);
        ::reslice->SetResliceAxesOrigin(img->GetCenter());
    }
    reslice->SetInterpolationModeToLinear();
    //reslice->SetOutputOrigin(0, 0, 0);
    //reslice->SetOutputExtent(0,400,0,400,0,400);
    //reslice->SetOutputSpacing(::spacing);
    ::reslice->Update(); // 没有此句的话在一开始不能显示三维线框

    //vtkNew<vtkImageChangeInformation> changer;
    ::changer = vtkSmartPointer<vtkImageChangeInformation>::New();
    changer->SetInputConnection(::reslice->GetOutputPort());
    //changer->SetOutputOrigin(0, 0, 0);
#if 1
    {
        auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
        {
            auto changer = reinterpret_cast<vtkImageChangeInformation*>(clientdata);
            auto origin = ::reslice->GetOutput()->GetOrigin();
            //auto bounds = img->GetBounds();
            auto axesOrigin = ::reslice->GetResliceAxesOrigin();
            changer->SetOutputOrigin(axesOrigin[0] + origin[0], axesOrigin[1] + origin[1], 0);
            //changer->SetOutputOrigin((bounds[1]-bounds[0])/2.+center[0], center[1] / 2, center[2] / 2);
            //changer->SetOutputOrigin(0-(bounds[1]-bounds[0])/2, 0-(bounds[3] - bounds[2]) / 2,0);
        };
        vtkNew<vtkCallbackCommand> pCC;
        pCC->SetCallback(f);
        pCC->SetClientData(changer.GetPointer());
        ::reslice->GetOutput()->AddObserver(vtkCommand::ModifiedEvent, pCC);
    }
#endif

    vtkNew<vtkImageActor> actor;
    actor->GetProperty()->SetColorWindow(6000);
    actor->GetProperty()->SetColorLevel(2000);
    actor->GetMapper()->SetInputConnection(changer->GetOutputPort());
    ren->AddActor(actor);

    //{
    //    vtkNew<vtkImageActor> actor;
    //    actor->GetMapper()->SetInputConnection(::reslice->GetOutputPort());
    //    ren->AddActor(actor);
    //}

    vtkNew<vtkActor> vrPtActor;
    vrPtActor->GetProperty()->SetPointSize(12);
    vrPtActor->GetProperty()->SetColor(1,0,0);
    ren->AddViewProp(vrPtActor);

    
    vrPtChangerActor->GetProperty()->SetPointSize(7);
    vrPtChangerActor->GetProperty()->SetColor(0, 1, 0);
    ren->AddViewProp(vrPtChangerActor);

    {
        auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
        {
            auto getIjkList = [](int extent[6])
            {
                return std::list<std::array<int, 3>>
                {
                    {extent[0], extent[2], extent[4]},
                    {extent[0], extent[2], extent[5]},
                    {extent[1], extent[2], extent[4]},
                    {extent[1], extent[2], extent[5]},
                    {extent[1], extent[3], extent[4]},
                    {extent[1], extent[3], extent[5]},
                    {extent[0], extent[3], extent[4]},
                    {extent[0], extent[3], extent[5]},
                };
            };

            ::getLogView()->Add("\n\nreslcie");
            {
                auto pActor = reinterpret_cast<vtkActor*>(clientdata);
                auto currSlice = ::reslice->GetOutput();
                auto extent = currSlice->GetExtent();
                vtkns::Pts_t pts;
                int ee[6];
                currSlice->GetExtent(ee);
                ::getLogView()->Add(fmt::format("ee: {}", ee));
                for (auto& i : getIjkList(extent))
                {
                    double xyz[3];
                    currSlice->TransformIndexToPhysicalPoint(i[0], i[1], i[2], xyz);
                    ::getLogView()->Add(fmt::format("ijk: {}", i));
                    ::getLogView()->Add(fmt::format("PhysicalPoint: {::.2f}", xyz));
                    // 转化为原始图像上的位置
                    double worldPt_original[4];
                    {
                        double pt[] = { xyz[0],xyz[1],xyz[2], 1 };
                        ::reslice->GetResliceAxes()->MultiplyPoint(pt, worldPt_original);
                    }

                    pts.push_back({ worldPt_original[0], worldPt_original[1], worldPt_original[2] });

                    //::getLogView()->Add(fmt::format("worldPt_img: {::.2f}", xyz));
                    ::getLogView()->Add(fmt::format("worldPt_original: {::.2f}\n", worldPt_original));
                }
                vtkns::makePoints(pts, pActor);
            }
            ::getLogView()->Add("\n\nchanger");
            {
                auto currSlice = ::changer->GetOutput();
                auto extent = currSlice->GetExtent();
                vtkns::Pts_t pts;
                int ee[6];
                currSlice->GetExtent(ee);
                ::getLogView()->Add(fmt::format("ee: {}", ee));
                for (auto& i : getIjkList(extent))
                {
                    double xyz[3];
                    currSlice->TransformIndexToPhysicalPoint(i[0], i[1], i[2], xyz);
                    ::getLogView()->Add(fmt::format("ijk: {}", i));
                    ::getLogView()->Add(fmt::format("PhysicalPoint_b: {::.2f}", xyz));
                    auto resliceOrigin = ::reslice->GetOutput()->GetOrigin();
                    xyz[0] += resliceOrigin[0];
                    xyz[1] += resliceOrigin[1];
                    xyz[2] += resliceOrigin[2];
                    ::getLogView()->Add(fmt::format("PhysicalPoint_a: {::.2f}", xyz));
                    // 转化为原始图像上的位置
                    double worldPt_original[4];
                    {
                        double pt[] = { xyz[0],xyz[1],xyz[2], 1 };
                        ::reslice->GetResliceAxes()->MultiplyPoint(pt, worldPt_original);
                    }

                    pts.push_back({ worldPt_original[0], worldPt_original[1], worldPt_original[2] });

                    //::getLogView()->Add(fmt::format("worldPt_img: {::.2f}", xyz));
                    ::getLogView()->Add(fmt::format("worldPt_original: {::.2f}\n", worldPt_original));
                }
                vtkns::makePoints(pts, vrPtChangerActor);
            }
        };
        vtkNew<vtkCallbackCommand> pCC;
        pCC->SetCallback(f);
        pCC->SetClientData(vrPtActor);
        ::changer->GetOutput()->AddObserver(vtkCommand::ModifiedEvent, pCC);
    }

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        static bool showResliceOutput = false;
        ImGui::Checkbox("ShowResliceAndChangerOutput", &showResliceOutput);
        if (showResliceOutput)
        {
            ImGui::Begin("ResliceOutput");
            vtkns::vtkObjSetup("Reslice", reslice->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("Changer", changer->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        }
        vtkns::vtkObjSetup("vtkImageActor", actor);
        vtkns::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("Changer", ::changer, ImGuiTreeNodeFlags_DefaultOpen);
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