#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    vtkNew<vtkMatrix4x4> matA, matB;

    {
        auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
        {
            vtkns::mat::genAxes(reinterpret_cast<vtkRenderer*>(clientdata), vtkMatrix4x4::SafeDownCast(caller));
        };
        vtkNew<vtkCallbackCommand> pCC;
        pCC->SetCallback(f);
        pCC->SetClientData(ren);
        matA->AddObserver(vtkCommand::ModifiedEvent, pCC);
    }

    {
        const double arr[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        matA->DeepCopy(arr);
        matB->DeepCopy(arr);
    }

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("A", matA, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("B", matB, ImGuiTreeNodeFlags_DefaultOpen);
        if (ImGui::Button("B*A"))
        {
            vtkNew<vtkMatrix4x4> mat;
            vtkMatrix4x4::Multiply4x4(matB, matA, mat);
            matA->DeepCopy(mat);
        }
        vtkns::HelpMarkerSameLine(u8R"(A沿着原始坐标系进行运动)");
        ImGui::SameLine();
        if (ImGui::Button("A*B"))
        {
            vtkNew<vtkMatrix4x4> mat;
            vtkMatrix4x4::Multiply4x4(matA, matB, mat);
            matA->DeepCopy(mat);
        }
        vtkns::HelpMarkerSameLine(u8R"(A沿着自身坐标系进行运动)");
    };

    AFTER_MY_CODE
}