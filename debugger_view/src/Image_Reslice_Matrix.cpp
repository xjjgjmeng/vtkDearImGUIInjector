#include <ImGuiCommon.h>

vtkSmartPointer<vtkImageReslice> reslice;

#define WHEEL_MODE_CENTER 0
#define WHEEL_MODE_MATRIX 1

auto wheelMode = WHEEL_MODE_CENTER;
#if 0
vtkNew<vtkActor> originActor;
vtkNew<vtkActor> xAxisActor, yAxisActor, zAxisActor;
vtkNew<vtkActor> xoyPlaneActor;
#endif

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static MyStyle* New() { return new MyStyle; }

  void OnMouseWheelForward() override
  {
      vtkns::mat::translate(::reslice->GetResliceAxes(), 0, 0, vtkImageData::SafeDownCast(::reslice->GetInput())->GetSpacing()[2] * -10, WHEEL_MODE_CENTER == wheelMode);
  }

  void OnMouseWheelBackward() override
  {
      vtkns::mat::translate(::reslice->GetResliceAxes(), 0, 0, vtkImageData::SafeDownCast(::reslice->GetInput())->GetSpacing()[2] * 10, WHEEL_MODE_CENTER == wheelMode);
  }
};

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);
    vtkns::genImgOutline(ren, img, false);
    //vtkns::genVR(ren, img, false, false);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);
#if 0
    {
        ::originActor->GetProperty()->SetColor(1, 1, 0);
        ::originActor->GetProperty()->SetPointSize(11);
        ::originActor->GetProperty()->SetRenderPointsAsSpheres(1);
        ren->AddViewProp(::originActor);

        struct
        {
            vtkActor* acotr;
            double color[3];
        } x{ xAxisActor.Get(), {1,0,0} }, y{ yAxisActor.Get(), {0,1,0} }, z{ zAxisActor.Get(), {0,0,1} };
        for (auto& [actor, color] : { x, y, z })
        {
            actor->GetProperty()->SetColor(const_cast<double*>(color));
            actor->GetProperty()->SetLineWidth(3);
            actor->GetProperty()->SetRenderLinesAsTubes(1);
            ren->AddViewProp(actor);
        }

        //::xoyPlaneActor->GetProperty()->SetOpacity(.8);
        ::xoyPlaneActor->GetProperty()->SetRepresentationToWireframe();
        ::xoyPlaneActor->GetProperty()->SetColor(0, 1, 0);
        ren->AddViewProp(::xoyPlaneActor);
    }
#endif
    {
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        ::reslice->SetResliceAxesDirectionCosines(x, y, z);
        {
            // 此刻ResliceAxes不为nullptr
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                {
#if 0
                    // origin
                    auto origin = ::reslice->GetResliceAxesOrigin();
                    vtkns::makePoints(vtkns::Pts_t{ {origin[0], origin[1], origin[2]} }, ::originActor);
                    // xyz
                    constexpr auto len = 180;
                    double oPt[4] = { 0,0,0,1 };
                    double xPt[4] = { len,0,0,1 };
                    double yPt[4] = { 0,len,0,1 };
                    double zPt[4] = { 0,0,len,1 };
                    for (auto& i : { oPt, xPt, yPt, zPt })
                    {
                        ::reslice->GetResliceAxes()->MultiplyPoint(i, i);
                    }
                    vtkns::makeLines(vtkns::Pts_t{ {oPt[0], oPt[1], oPt[2]}, {xPt[0], xPt[1], xPt[2]} }, xAxisActor);
                    vtkns::makeLines(vtkns::Pts_t{ {oPt[0], oPt[1], oPt[2]}, {yPt[0], yPt[1], yPt[2]} }, yAxisActor);
                    vtkns::makeLines(vtkns::Pts_t{ {oPt[0], oPt[1], oPt[2]}, {zPt[0], zPt[1], zPt[2]} }, zAxisActor);
                    // xoy
                    vtkNew<vtkPlaneSource> src;
                    src->SetPoint1(xPt);
                    src->SetPoint2(yPt);
                    src->SetOrigin(oPt);
                    src->SetCenter(oPt);
                    src->SetXResolution(33);
                    src->SetYResolution(33);
                    vtkNew<vtkPolyDataMapper> mapper;
                    mapper->SetInputConnection(src->GetOutputPort());
                    ::xoyPlaneActor->SetMapper(mapper);
#endif
                vtkns::mat::genAxes(reinterpret_cast<vtkRenderer*>(clientdata), ::reslice->GetResliceAxes());
                };
            vtkNew<vtkCallbackCommand> pCC;
            pCC->SetCallback(f);
            pCC->SetClientData(ren);
            ::reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, pCC);
        }
        ::reslice->SetResliceAxesOrigin(img->GetCenter()[0], img->GetCenter()[1], img->GetCenter()[2]);
        //::reslice->SetResliceAxesOrigin(0.1, 0, 0); // 无论设置到中点还是0处都不影响使用CENTER方式滚动，相当于进行了平移变换
    }

    {
        vtkNew<vtkImageActor> actor;
        actor->GetProperty()->SetColorLevel(2200);
        actor->GetProperty()->SetColorWindow(6500);
#if 1
        vtkNew<vtkImageChangeInformation> changer;
        changer->SetOriginTranslation(-100, 0, 0); // 偏移一些距离方便查看切面和结果对比
        changer->SetInputConnection(reslice->GetOutputPort());
        actor->GetMapper()->SetInputConnection(changer->GetOutputPort());
#else
        actor->GetMapper()->SetInputConnection(::reslice->GetOutputPort());
#endif
        ren->AddActor(actor);
    }

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        {
            ImGui::Text("WheelMode"); ImGui::SameLine();
            ImGui::RadioButton("CENTER", &wheelMode, WHEEL_MODE_CENTER); ImGui::SameLine();
            ImGui::RadioButton("MATRIX", &wheelMode, WHEEL_MODE_MATRIX);
            ImGui::SameLine(); vtkns::HelpMarker(u8R"(使用MATRIX只修改了[原坐标系]的Z坐标，使用CENTER沿着[新坐标系]的Z移动)");
        }
        ImGui::Text(vtkns::getMatrixString(::reslice->GetResliceAxes()).c_str());
        vtkns::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    auto style = vtkSmartPointer<MyStyle>::New();
    rwi->SetInteractorStyle(style);
    rwi->EnableRenderOff();
    rwi->Start();
}