#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    {
        vtkNew<vtkRegularPolygonSource> src;
        src->GeneratePolygonOn();
        src->SetNumberOfSides(360);
        src->SetRadius(2);
        src->SetCenter(0, 0, 0);
        
        vtkNew<vtkLinearExtrusionFilter> filter;
        filter->SetInputConnection(src->GetOutputPort());
#if 1
        filter->SetExtrusionTypeToPointExtrusion();
        filter->SetExtrusionPoint(0,0,5);
        filter->SetScaleFactor(0.2);
#endif

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(filter->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        //actor->SetPosition(11, 0, 0);
        ren->AddActor(actor);
    }

    {
        vtkNew<vtkPoints> points;
        vtkNew<vtkCellArray> lines;
        vtkNew<vtkPolyData> profile;
        points->InsertNextPoint(0, 0, 0);
        points->InsertNextPoint(2, 0, 0);
        points->InsertNextPoint(2, 0, 1);
        points->InsertNextPoint(0, 0, 1);
        lines->InsertNextCell(4);
        lines->InsertCellPoint(0);
        lines->InsertCellPoint(1);
        lines->InsertCellPoint(2);
        lines->InsertCellPoint(3);
        profile->SetPoints(points);
        profile->SetLines(lines);

        vtkNew<vtkRotationalExtrusionFilter> filter;
        filter->SetInputData(profile);
        filter->SetAngle(360);
        filter->SetResolution(360);

        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(filter->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetPosition(5, 0, 0);
        ren->AddActor(actor);
    }

    {
        vtkNew<vtkVectorText> text;
        text->SetText("vtk");
        vtkNew<vtkLinearExtrusionFilter> filter;
        filter->SetInputConnection(text->GetOutputPort());
        filter->SetVector(0, 0, 1);
        filter->SetExtrusionTypeToVectorExtrusion();
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(filter->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetPosition(9, 0, 0);
        ren->AddActor(actor);
    }

    {
        vtkNew<vtkRegularPolygonSource> src;
        src->GeneratePolygonOn();
        src->SetNumberOfSides(360);
        src->SetRadius(0.2);
        src->SetCenter(1, 0, 0);
        src->SetNormal(0, 1, 0);

        vtkNew<vtkRotationalExtrusionFilter> filter;
        filter->SetInputConnection(src->GetOutputPort());
        filter->SetAngle(2160);
        filter->SetResolution(360);
        filter->SetTranslation(6);

        vtkNew<vtkPolyDataNormals> normals;
        normals->SetInputConnection(filter->GetOutputPort());
        normals->SetFeatureAngle(60);

        vtkns::renderPolydata(ren, normals->GetOutputPort()).first->SetPosition(15, 0, 0);
    }

    ren->ResetCamera();

    ::imgui_render_callback = [&]
    {
        //vtkns::vtkObjSetup("Data", data, ImGuiTreeNodeFlags_DefaultOpen);
        //vtkns::vtkObjSetup("Actor", actor);
    };

    AFTER_MY_CODE
}