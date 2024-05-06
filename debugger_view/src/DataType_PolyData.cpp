#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    //vtkns::labelWorldZero(ren);

    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> lines;
    vtkNew<vtkPolyData> poly;

    points->InsertNextPoint(0, 0, 0);
    points->InsertNextPoint(0, 2, 0);
    points->InsertNextPoint(2, 4, 0);
    points->InsertNextPoint(4, 4, 0);
    lines->InsertNextCell(4);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(1);
    lines->InsertCellPoint(2);
    lines->InsertCellPoint(3);
    poly->SetPoints(points);
    poly->SetLines(lines);
    //poly->SetVerts(lines);
    //poly->SetStrips(lines);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(poly);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    ren->ResetCamera();

    ::imgui_render_callback = [&]
    {
        //vtkns::vtkObjSetup("Data", data, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("Actor", actor);
    };

    AFTER_MY_CODE
}