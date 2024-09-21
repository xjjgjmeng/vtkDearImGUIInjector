#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    vtkns::labelWorldZero(ren, false, 0.1, 1);

    vtkNew<vtkConeSource> coneSource;
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(coneSource->GetOutputPort());
    vtkNew<vtkActor> coneActor;
    coneActor->SetMapper(mapper);
    ren->AddActor(coneActor);

    auto textActor = vtkns::genTextActor();
    textActor->SetInput("Hello");
    ren->AddActor(textActor);

    vtkNew<vtkActor2D> ptsActor;
    ptsActor->GetProperty()->SetPointSize(8);
    //ptsActor->GetProperty()->SetRenderPointsAsSpheres(1);
    ptsActor->GetProperty()->SetColor(1, 0, 0);
    //ren->AddActor(ptsActor);

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("ConeActor", coneActor, ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("TextActor", textActor, ImGuiTreeNodeFlags_DefaultOpen);

        textActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
        double pos[3];
        textActor->GetPositionCoordinate()->GetValue(pos);
        double sz[2];
        textActor->GetSize(::pRen, sz);
        double pos_display[3];
        vtkInteractorObserver::ComputeWorldToDisplay(::pRen, pos[0], pos[1], pos[2], pos_display);
        double pos2[4];
        vtkInteractorObserver::ComputeDisplayToWorld(::pRen, pos_display[0] + sz[0], pos_display[1] + sz[1], 0, pos2);
        vtkns::makePoints({ {pos[0], pos[1], pos[2]} ,{pos2[0], pos2[1], pos2[2]}}, ptsActor);
        ::pRen->AddActor(ptsActor);
    };

    AFTER_MY_CODE
}