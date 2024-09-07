#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);
    auto volume = vtkns::genVR(ren, img, false, true);

    vtkNew<vtkImageReslice> reslice;
    reslice->SetInputData(img);
    reslice->SetOutputDimensionality(2);
    reslice->SetInterpolationModeToLinear();
    reslice->SetOutputSpacing(img->GetSpacing());

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);

    {
        reslice->SetResliceAxesDirectionCosines(1, 0, 0, 0, 1, 0, 0, 0, 1);
#if 1 // 显示切割平面
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
#endif
        reslice->SetResliceAxesOrigin(img->GetCenter());
    }
#if 0
    reslice->SetOutputOrigin(0, 0, 0);
    reslice->SetOutputExtent(0, 400, 0, 400, 0, 400);
#endif

    vtkNew<vtkImageActor> actor;
    vtkNew<vtkImageChangeInformation> changer;
    changer->SetInputConnection(reslice->GetOutputPort());
    actor->GetMapper()->SetInputConnection(changer->GetOutputPort());
    actor->GetProperty()->SetColorWindow(6000);
    actor->GetProperty()->SetColorLevel(2000);
    ren->AddActor(actor);

    ::imgui_render_callback = [&]
        {
            if (bool v = volume->GetVisibility(); ImGui::Checkbox("ShowVolume", &v))
            {
                volume->SetVisibility(v);
            }
            vtkns::vtkObjSetup("OriginalImageData", img);
            vtkns::vtkObjSetupWin("ResliceOutput", reslice->GetOutput());
            vtkns::vtkObjSetupWin("ChangerOutput", changer->GetOutput());
            vtkns::vtkObjSetup("Reslice", reslice, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("Changer", changer, ImGuiTreeNodeFlags_DefaultOpen);
            if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx(u8"Post处理", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static bool autoOrigin0 = false;
                ImGui::Checkbox("AutoOrigin0", &autoOrigin0); vtkns::HelpMarkerSameLine(u8"自动调整输出影像的origin，使左下角永远在0处，用于没有设置输出Origin和Extent");
                if (autoOrigin0)
                {
                    const auto origin = reslice->GetOutput()->GetOrigin();
                    changer->SetOriginTranslation(-origin[0], -origin[1], -origin[2]);
                }
            }
        };

    AFTER_MY_CODE
}