#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkPoints> points;
    points->InsertNextPoint(0, 0, 0);
    points->InsertNextPoint(0, 2, 0);
    points->InsertNextPoint(2, 4, 0);
    points->InsertNextPoint(4, 4, 0);
    vtkNew<vtkCellArray> lines;
    lines->InsertNextCell(4);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(1);
    lines->InsertCellPoint(2);
    lines->InsertCellPoint(3);
    vtkNew<vtkPolyData> profile;
    profile->SetPoints(points);
    profile->SetLines(lines);

    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(profile);

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        ren->AddActor(actor);
    }

    {
        vtkNew<vtkTubeFilter> filter;
        filter->SetInputData(profile);
        filter->SetRadius(.5);
        filter->SetNumberOfSides(13);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(filter->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        ren->AddActor(actor);
    }

    {
        vtkNew<vtkImplicitModeller> imp;
        imp->SetInputData(profile);
        imp->SetSampleDimensions(100, 100, 100);
        imp->SetMaximumDistance(0.25);
        imp->SetModelBounds(-10, 10, -10, 10, -10, 10);
        vtkNew<vtkContourFilter> filter;
        filter->SetInputConnection(imp->GetOutputPort());
        filter->SetValue(0, .5);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(filter->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetPosition(5, 0, 0);
        ren->AddActor(actor);
    }

    {
        vtkNew<vtkRegularPolygonSource> src;
        src->GeneratePolygonOn();
        src->SetNumberOfSides(50);
        src->SetRadius(2);
        src->SetCenter(0, 0, 0);
        vtkNew<vtkImplicitModeller> imp;
        imp->SetInputConnection(src->GetOutputPort());
        imp->SetSampleDimensions(100, 100, 8);
        //imp->SetMaximumDistance(0.25);
        imp->SetModelBounds(-3, 3, -3, 3, -.2, .2);
        vtkNew<vtkContourFilter> filter;
        filter->SetInputConnection(imp->GetOutputPort());
        filter->SetValue(0, .2);
        vtkNew<vtkWarpTo> wt;
        wt->SetInputConnection(filter->GetOutputPort());
        wt->SetPosition(0, 0, 5);
        wt->SetScaleFactor(0.85);
        wt->AbsoluteOn();
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(wt->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetPosition(11, 0, 0);
        ren->AddActor(actor);
    }

    ren->ResetCamera();

    ::imgui_render_callback = [&]
    {
        //vtkns::vtkObjSetup("Data", data, ImGuiTreeNodeFlags_DefaultOpen);
        //vtkns::vtkObjSetup("Actor", actor);
    };

    AFTER_MY_CODE
}