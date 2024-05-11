#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    auto reslice = vtkSmartPointer<vtkImageReslice>::New();
    reslice->SetInputData(img);
    reslice->SetOutputDimensionality(3);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);
    // 将reslice得到的image用线框动态表示出来
    {
        auto actor = vtkns::genImgOutline(ren, reslice->GetOutput(), true);
        actor->GetProperty()->SetColor(1., 0., 0.);
        actor->GetProperty()->SetLineWidth(5);
    }

    vtkns::genVR(ren, img, false, true);
    // 将切割出来的体数据渲染出来
    vtkns::genVR(ren, reslice->GetOutput(), false);
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

    {
        const double x[3] = { 1,0,0 };
        const double y[3] = { 0,1,0 };
        const double z[3] = { 0,0,1 };
        const double o[3] = { 0,0,0 };
        reslice->SetResliceAxesDirectionCosines(x, y, z);
        reslice->SetResliceAxesOrigin(img->GetCenter());
    }
    reslice->SetInterpolationModeToLinear();
    reslice->SetOutputOrigin(0, 0, 0);
    reslice->SetOutputExtent(0,400,0,400,0,400);
    reslice->SetOutputSpacing(img->GetSpacing());
    reslice->Update(); // 没有此句的话在一开始不能显示三维线框

    vtkNew<vtkImageActor> actor;
    actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
    ren->AddActor(actor);

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("OriginalImageData", img);
        static bool showResliceOutput = false;
        ImGui::Checkbox("ShowResliceOutput", &showResliceOutput);
        if (showResliceOutput)
        {
            ImGui::Begin("ResliceOutput");
            vtkns::vtkObjSetup("ImageData", reslice->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
        }
        vtkns::vtkObjSetup("Reslice", reslice, ImGuiTreeNodeFlags_DefaultOpen);
    };

    AFTER_MY_CODE
}