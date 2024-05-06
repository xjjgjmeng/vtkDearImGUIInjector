#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
        //vtkns::labelWorldZero(ren);

    std::array<std::array<double, 3>, 8> pts = {
        {
            {{0,0,0}},
            {{1,0,0}},
            {{1,1,0}},
            {{0,1,0}},
            {{0,0,1}},
            {{1,0,1}},
            {{1,1,1}},
            {{0,1,1}}
        }
    };
    std::array<std::array<vtkIdType, 4>, 6> ordering = {
        {
            {{0,1,2,3}},
            {{4,5,6,7}},
            {{0,1,5,4}},
            {{1,2,6,5}},
            {{2,3,7,6}},
            {{3,0,4,7}},
        }
    };

    vtkNew<vtkPolyData> cube;
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> polys;
    vtkNew<vtkFloatArray> scalars;
    for (std::size_t i = 0; i < pts.size(); ++i)
    {
        points->InsertPoint(i, pts[i].data());
        scalars->InsertTuple1(i, i);
    }
    for (auto&& i : ordering)
    {
        polys->InsertNextCell(vtkIdType(i.size()), i.data());
    }
    cube->SetPoints(points);
    cube->SetPolys(polys);
    cube->GetPointData()->SetScalars(scalars);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(cube);
    mapper->SetScalarRange(cube->GetScalarRange());

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