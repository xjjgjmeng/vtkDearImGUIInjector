#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    // vtkns::labelWorldZero(ren);

    vtkNew<vtkSphereSource> src;
    src->SetThetaResolution(51);
    src->SetPhiResolution(17);

    vtkNew<vtkElevationFilter> elevationF;
    elevationF->SetInputConnection(src->GetOutputPort());
    elevationF->SetLowPoint(0, 0, -0.5);
    elevationF->SetHighPoint(0, 0, 0.5);
    elevationF->SetScalarRange(-1, 1);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetScalarRange(-1, 1);
    mapper->SetInputConnection(elevationF->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    vtkNew<vtkScalarBarActor> bar;
    bar->SetLookupTable(mapper->GetLookupTable());
    //bar->SetMaximumHeightInPixels(500);
    ren->AddActor2D(bar);

    ren->ResetCamera();

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("elevation", elevationF, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("SphereSource", src);
            vtkns::vtkObjSetup("actor", actor);
            vtkns::vtkObjSetup("bar", bar);
        };

    AFTER_MY_CODE
}