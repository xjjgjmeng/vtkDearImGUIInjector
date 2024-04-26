#include <ImGuiCommon.h>

vtkSmartPointer<vtkImageReslice> reslice;

#define WHEEL_MODE_CENTER 0
#define WHEEL_MODE_MATRIX 1

auto wheelMode = WHEEL_MODE_CENTER;
vtkNew<vtkActor> originActor;
vtkNew<vtkActor> xAxisActor, yAxisActor, zAxisActor;
vtkNew<vtkActor> xoyPlaneActor;

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static MyStyle* New() { return new MyStyle; }

  void OnMouseWheelForward() override
  {
      resliceImg(-10);
  }

  void OnMouseWheelBackward() override
  {
      resliceImg(10);
  }

private:
    void resliceImg(const int factor)
    {
        if (WHEEL_MODE_CENTER == wheelMode)
        {
            // move the center point that we are slicing through
            double point[4];
            point[0] = 0.0;
            point[1] = 0.0;
            point[2] = vtkImageData::SafeDownCast(::reslice->GetInput())->GetSpacing()[2] * factor;
            point[3] = 1.0;
            double center[4];
            vtkMatrix4x4* matrix = ::reslice->GetResliceAxes();
            matrix->MultiplyPoint(point, center);
            matrix->SetElement(0, 3, center[0]);
            matrix->SetElement(1, 3, center[1]);
            matrix->SetElement(2, 3, center[2]);
        }
        else
        {
            const double arr[] = {
                1.,0.,0., 0.,
                0.,1.,0., 0.,
                0.,0.,1., vtkImageData::SafeDownCast(::reslice->GetInput())->GetSpacing()[2] * factor,
                0.,0.,0.,1.
            };
            double r[16];
            vtkMatrix4x4::Multiply4x4(arr, ::reslice->GetResliceAxes()->GetData(), r);
            ::reslice->GetResliceAxes()->DeepCopy(r);
        }
    }
};

int main()
{
    SETUP_WINDOW
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);
    vtkns::genImgOutline(ren, img, false);
    vtkns::genVR(ren, img, false, false);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);

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
        ren->AddViewProp(::xoyPlaneActor);
    }

    {
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        ::reslice->SetResliceAxesDirectionCosines(x, y, z);
        {
            // 此刻ResliceAxes不为nullptr
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                {
                    auto img = vtkImageData::SafeDownCast(::reslice->GetInput());

                    auto origin = ::reslice->GetResliceAxesOrigin();
                    vtkns::makePoints(vtkns::Pts_t{ {origin[0], origin[1], origin[2]} }, ::originActor);

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
                    vtkNew<vtkPolyDataMapper> mapper;
                    mapper->SetInputConnection(src->GetOutputPort());
                    ::xoyPlaneActor->SetMapper(mapper);
                };
            vtkNew<vtkCallbackCommand> pCC;
            pCC->SetCallback(f);
            ::reslice->GetResliceAxes()->AddObserver(vtkCommand::ModifiedEvent, pCC);
        }
        ::reslice->SetResliceAxesOrigin(img->GetCenter()[0], img->GetCenter()[1], img->GetCenter()[2]);
        //::reslice->SetResliceAxesOrigin(0, 0, 0);
    }

    vtkNew<vtkImageActor> actor;
    actor->GetProperty()->SetColorLevel(2200);
    actor->GetProperty()->SetColorWindow(6500);
    actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
    ren->AddActor(actor);

    ::pWindow = rw;
    ::imgui_render_callback = [&]
    {
        {
            ImGui::Text("WheelMode"); ImGui::SameLine();
            ImGui::RadioButton("CENTER", &wheelMode, WHEEL_MODE_CENTER); ImGui::SameLine();
            ImGui::RadioButton("MATRIX", &wheelMode, WHEEL_MODE_MATRIX);
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