#include <ImGuiCommon.h>

vtkSmartPointer<vtkImageReslice> reslice;

#define WHEEL_MODE_CENTER 0
#define WHEEL_MODE_MATRIX 1

auto wheelMode = WHEEL_MODE_CENTER;

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
    rwi->SetInteractorStyle(vtkNew<MyStyle>{});
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);
    vtkns::genImgOutline(ren, img, false);
    vtkns::genVR(ren, img, false, false);

    ::reslice = vtkSmartPointer<vtkImageReslice>::New();
    ::reslice->SetInputData(img);
    ::reslice->SetOutputDimensionality(2);

    {
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        ::reslice->SetResliceAxesDirectionCosines(x, y, z);
        {
            // 此刻ResliceAxes不为nullptr
            auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
                {
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

    vtkNew<vtkImageActor> actor;
    actor->GetProperty()->SetColorLevel(2200);
    actor->GetProperty()->SetColorWindow(6500);
#if 1
    vtkNew<vtkImageChangeInformation> changer;
    //changer->SetOriginTranslation(-100, 0, 0); // 偏移一些距离方便查看切面和结果对比
    changer->SetInputConnection(reslice->GetOutputPort());
    actor->GetMapper()->SetInputConnection(changer->GetOutputPort());
#else
    actor->GetMapper()->SetInputConnection(::reslice->GetOutputPort());
#endif
    ren->AddActor(actor);

    ::imgui_render_callback = [&]
    {
        {
            ImGui::Text("WheelMode"); ImGui::SameLine();
            ImGui::RadioButton("CENTER", &wheelMode, WHEEL_MODE_CENTER); ImGui::SameLine();
            ImGui::RadioButton("MATRIX", &wheelMode, WHEEL_MODE_MATRIX);
            ImGui::SameLine(); vtkns::HelpMarker(u8R"(使用MATRIX只修改了[原坐标系]的Z坐标，使用CENTER沿着[新坐标系]的Z移动)");
        }

        static bool showResliceOutput = false;
        static bool showChangerOutput = false;
        ImGui::Checkbox("ShowResliceOutput", &showResliceOutput);
        ImGui::SameLine();
        ImGui::Checkbox("ShowChangerOutput", &showChangerOutput);

        ImGui::Text(vtkns::getMatrixString(::reslice->GetResliceAxes()).c_str());
        vtkns::vtkObjSetup("Reslice", ::reslice, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("Changer", changer);

        if (showResliceOutput)
        {
            ImGui::Begin("ResliceOutput");
            vtkns::vtkObjSetup("ImageData", ::reslice->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        }

        if (showChangerOutput)
        {
            ImGui::Begin("ChangerOutput");
            vtkns::vtkObjSetup("ImageData", changer->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        }
    };

    AFTER_MY_CODE
}