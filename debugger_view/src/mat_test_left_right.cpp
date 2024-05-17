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
        double arr[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        matA->DeepCopy(arr);
    }
    {
        double arr[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
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
        ImGui::SameLine();
        if (ImGui::Button("A*B"))
        {
            vtkNew<vtkMatrix4x4> mat;
            vtkMatrix4x4::Multiply4x4(matA, matB, mat);
            matA->DeepCopy(mat);
        }
    };

    AFTER_MY_CODE
}