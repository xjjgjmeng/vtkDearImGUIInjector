#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    vtkns::labelWorldZero(ren);

    vtkNew<vtkSphereSource> src;
    src->SetRadius(10);
    src->SetPhiResolution(10);
    src->SetThetaResolution(10);

    vtkNew<vtkPlane> plane;
    plane->SetNormal(0, 0, 1);
    plane->SetOrigin(0, 0, 0);

    vtkNew<vtkCutter> cutter;
    cutter->SetInputConnection(src->GetOutputPort());
    cutter->SetGenerateCutScalars(0);
    cutter->SetGenerateTriangles(0);
    cutter->SetCutFunction(plane);

    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(src->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetOpacity(0.1);
        ren->AddViewProp(actor);
    }

    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(cutter->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        ren->AddViewProp(actor);
    }

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("Cutter", cutter, ImGuiTreeNodeFlags_DefaultOpen);
        //vtkns::vtkObjSetup("B", matB, ImGuiTreeNodeFlags_DefaultOpen);
    };

    AFTER_MY_CODE
}