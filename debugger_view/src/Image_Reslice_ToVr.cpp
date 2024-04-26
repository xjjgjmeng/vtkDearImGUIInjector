#include <ImGuiCommon.h>
#include <PolyDataHelper.h>

vtkSmartPointer<vtkImageReslice> reslice;
vtkSmartPointer<vtkImageChangeInformation> changer;
vtkNew<vtkActor> vrChangerPtActor;
vtkNew<vtkActor> vrResliceLineActor;
vtkNew<vtkActor> vrReslicePtActor;
vtkSmartPointer<vtkImageData> img;

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyStyle* New()
    {
        return new MyStyle;
    }

    MyStyle()
    {
        this->m_Ptplacer->SetProjectionNormalToOblique();
        {
            vtkNew<vtkPlane> plane;
            plane->SetOrigin(0, 0, 0);
            plane->SetNormal(0, 0, 1);
            this->m_Ptplacer->SetObliquePlane(plane);
        }
    }

    void OnLeftButtonDown() override
    {
        if (!this->Interactor->GetRepeatCount())
        {
            __super::OnLeftButtonDown();
            return;
        }

        vtkns::Pt_t pt;

        int eventPt[2];
        this->Interactor->GetEventPosition(eventPt);
        const auto pRenderer = this->Interactor->FindPokedRenderer(eventPt[0], eventPt[1]);

        {
            double worldOrient[9];
            double eventPt_d[] = { eventPt[0], eventPt[1] };
            this->m_Ptplacer->ComputeWorldPosition(pRenderer, eventPt_d, pt.data(), worldOrient);
        }

        ::logger()->Add("ChangerActorPt: {}", pt);

        auto originTranslation = ::changer->GetOriginTranslation();
        pt[0] -= originTranslation[0];
        pt[1] -= originTranslation[1];
        pt[2] -= originTranslation[2];

        // 转化为原始图像上的位置
        double worldPt_original[4];
        {
            double pt_[] = { pt[0],pt[1],pt[2], 1 };
            ::reslice->GetResliceAxes()->MultiplyPoint(pt_, worldPt_original);
        }

        ::logger()->Add("OriginPt: {}", worldPt_original);

        this->m_pts.push_back({ worldPt_original[0], worldPt_original[1], worldPt_original[2] });
        vtkns::makePoints(this->m_pts, this->m_actor);
        pRenderer->AddViewProp(this->m_actor);
        this->m_actor->GetProperty()->SetPointSize(25);
        this->Interactor->Render();
    }

private:
    vtkNew<vtkBoundedPlanePointPlacer> m_Ptplacer;
    vtkns::Pts_t m_pts;
    vtkNew<vtkActor> m_actor;
};

int main()
{
    SETUP_WINDOW
    img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false);
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
    ::reslice->Update(); // 没有此句的话在一开始不能显示三维线框

    ::changer = vtkSmartPointer<vtkImageChangeInformation>::New();
    ::changer->SetInputConnection(::reslice->GetOutputPort());

    // 用于显示reslice生成的影像
    vtkNew<vtkImageActor> resliceImgActor;
    resliceImgActor->GetProperty()->SetColorWindow(6000);
    resliceImgActor->GetProperty()->SetColorLevel(2000);
    resliceImgActor->GetMapper()->SetInputConnection(::reslice->GetOutputPort());
    ren->AddActor(resliceImgActor);
    resliceImgActor->VisibilityOff();

    // 用于显示changer生成的影像
    vtkNew<vtkImageActor> changerImgActor;
    changerImgActor->GetProperty()->SetColorWindow(6000);
    changerImgActor->GetProperty()->SetColorLevel(2000);
    changerImgActor->GetMapper()->SetInputConnection(::changer->GetOutputPort());
    ren->AddActor(changerImgActor);

    vrReslicePtActor->GetProperty()->SetPointSize(12);
    vrReslicePtActor->GetProperty()->SetColor(1,0,0);
    ren->AddViewProp(vrReslicePtActor);

    vrResliceLineActor->GetProperty()->SetColor(1, 1, 0);
    vrResliceLineActor->GetProperty()->SetRepresentationToSurface();
    ren->AddViewProp(vrResliceLineActor);

    vrChangerPtActor->GetProperty()->SetPointSize(7);
    vrChangerPtActor->GetProperty()->SetColor(0, 1, 0);
    ren->AddViewProp(vrChangerPtActor);

    {
        auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
        {
            auto ijkList = [](int extent[6])
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
            ::logger()->Add("\n\nreslice");
            {
                //auto pActor = reinterpret_cast<vtkActor*>(clientdata);
                auto currSlice = ::reslice->GetOutput();
                int extent[6];
                currSlice->GetExtent(extent);
                ::logger()->Add("extent: {}", extent);
                vtkns::Pts_t pts;
                for (auto& i : ijkList(extent))
                {
                    double xyz[3];
                    currSlice->TransformIndexToPhysicalPoint(i[0], i[1], i[2], xyz);
                    ::logger()->Add("ijk: {}", i);
                    ::logger()->Add("PhysicalPoint: {::.2f}", xyz);
                    // 转化为原始图像上的位置
                    double worldPt_original[4];
                    {
                        double pt[] = { xyz[0],xyz[1],xyz[2], 1 };
                        ::reslice->GetResliceAxes()->MultiplyPoint(pt, worldPt_original);
                    }

                    pts.push_back({ worldPt_original[0], worldPt_original[1], worldPt_original[2] });

                    //::getLogView()->Add(fmt::format("worldPt_img: {::.2f}", xyz));
                    //::getLogView()->Add(fmt::format("worldPt_original: {::.2f}\n", worldPt_original));
                }
                vtkns::makePoints(pts, ::vrReslicePtActor);
                vtkns::makeLines(pts, ::vrResliceLineActor);
            }
            ::logger()->Add("\n\nchanger");
            {
                auto currSlice = ::changer->GetOutput();
                vtkns::Pts_t pts;
                int extent[6];
                currSlice->GetExtent(extent);
                ::logger()->Add("extent: {}", extent);
                for (auto& i : ijkList(extent))
                {
                    double xyz[3];
                    currSlice->TransformIndexToPhysicalPoint(i[0], i[1], i[2], xyz);
                    ::logger()->Add("ijk: {}", i);
                    ::logger()->Add("PhysicalPoint_b: {::.2f}", xyz);
                    //auto resliceOrigin = ::reslice->GetOutput()->GetOrigin();
                    double originTranslation[3];
                    ::changer->GetOriginTranslation(originTranslation);
                    ::logger()->Add("OriginTranslation: {}", originTranslation);
                    xyz[0] -= originTranslation[0];
                    xyz[1] -= originTranslation[1];
                    xyz[2] -= originTranslation[2];
                    ::logger()->Add("PhysicalPoint_a: {::.2f}", xyz);
                    // 转化为原始图像上的位置
                    double worldPt_original[4];
                    {
                        double pt[] = { xyz[0],xyz[1],xyz[2], 1 };
                        ::reslice->GetResliceAxes()->MultiplyPoint(pt, worldPt_original);
                    }

                    pts.push_back({ worldPt_original[0], worldPt_original[1], worldPt_original[2] });

                    //::getLogView()->Add(fmt::format("worldPt_img: {::.2f}", xyz));
                    //::getLogView()->Add(fmt::format("worldPt_original: {::.2f}\n", worldPt_original));
                }
                vtkns::makePoints(pts, ::vrChangerPtActor);
            }
        };
        vtkNew<vtkCallbackCommand> pCC;
        pCC->SetCallback(f);
        //pCC->SetClientData(vrPtActor);
        ::changer->GetOutput()->AddObserver(vtkCommand::ModifiedEvent, pCC);
    }

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        static bool showResliceOutput = false;
        ImGui::Checkbox("ShowResliceAndChangerOutput", &showResliceOutput);
        ImGui::SameLine();
        if (bool v = resliceImgActor->GetVisibility(); ImGui::Checkbox("ShowResliceImg", &v)) resliceImgActor->SetVisibility(v);
        if (showResliceOutput)
        {
            ImGui::Begin("ResliceOutput");
            vtkns::vtkObjSetup("Reslice", reslice->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("Changer", changer->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        }
        vtkns::vtkObjSetup("vtkImageActor", changerImgActor);
        vtkns::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("Changer", ::changer, ImGuiTreeNodeFlags_DefaultOpen);
        {
            auto f = [](const int idx, const int v)
                {
                    ::reslice->GetResliceAxes()->SetElement(idx, 3, ::reslice->GetResliceAxes()->GetElement(idx, 3) + v);
                    double ot[3];
                    ::changer->GetOriginTranslation(ot);
                    ot[idx] += v;
                    ::changer->SetOriginTranslation(ot);
                };
            if (ImGui::Button("X+")) f(0, 1); ImGui::SameLine();
            if (ImGui::Button("X-")) f(0, -1); ImGui::SameLine();
            if (ImGui::Button("Y+")) f(1, 1); ImGui::SameLine();
            if (ImGui::Button("Y-")) f(1, -1);/* ImGui::SameLine();
            if (ImGui::Button("Z+")) f(2, 1); ImGui::SameLine();
            if (ImGui::Button("Z-")) f(2, -1);*/
        }
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
    //vtkInteractorStyleSwitch::SafeDownCast(rwi->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    rwi->SetInteractorStyle(vtkNew<MyStyle>{});
    rwi->EnableRenderOff();
    rwi->Start();
}