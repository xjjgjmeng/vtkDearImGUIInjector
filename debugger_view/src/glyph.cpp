#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    vtkns::labelWorldZero(ren, false);

    vtkNew<vtkPoints> points;
    points->InsertNextPoint(0, 0, 0);
    points->InsertNextPoint(2, 2, 2);

    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(points);

    vtkNew<vtkGlyphSource2D> circleSource;
    circleSource->SetGlyphTypeToCircle();
    circleSource->SetScale(3);

    // 创建一个 Glyph2D 映射器
    vtkNew<vtkGlyph2D> glyph2D;
    glyph2D->SetSourceConnection(circleSource->GetOutputPort());
    glyph2D->SetInputData(polyData);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(glyph2D->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    //actor->GetProperty()->SetColor(1, 0, 0);
    pRen->AddActor(actor);

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("GlyphSource2D", circleSource, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("Glyph2D", glyph2D, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("Actor", actor);
        };

    AFTER_MY_CODE
}