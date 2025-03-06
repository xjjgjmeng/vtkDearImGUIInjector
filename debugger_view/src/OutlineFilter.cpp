#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkActor> ptsActor;
    ptsActor->GetProperty()->SetColor(1, 1, 0);
    ptsActor->GetProperty()->SetPointSize(8);
    ren->AddActor(ptsActor);

    vtkNew<vtkActor> outlineActor;
    ren->AddActor(outlineActor);

    double pts[3][3] =
    {
        {0., 0., 0.},
        {10., 10., 10.},
        {10., 0., 10.}
    };

    auto fn = [&]
        {
            auto getPts = [&]
                {
                    return vtkns::Pts_t{ { pts[0][0],pts[0][1],pts[0][2] }, { pts[1][0],pts[1][1],pts[1][2] }, { pts[2][0],pts[2][1],pts[2][2] } };
                };
            vtkns::makePoints(getPts(), ptsActor);
            vtkNew<vtkOutlineFilter> outline;
            outline->SetInputData(vtkns::makePoints(getPts()));
            vtkNew<vtkPolyDataMapper> outlineMapper;
            outlineMapper->SetInputConnection(outline->GetOutputPort());
            outlineActor->SetMapper(outlineMapper);
        };

    fn();

    ::imgui_render_callback = [&]
        {
            if (ImGui::DragScalarN("pt0", ImGuiDataType_Double, pts[0], IM_ARRAYSIZE(pts[0]), 1.f) ||
                ImGui::DragScalarN("pt1", ImGuiDataType_Double, pts[1], IM_ARRAYSIZE(pts[1]), 1.f) ||
                ImGui::DragScalarN("pt2", ImGuiDataType_Double, pts[2], IM_ARRAYSIZE(pts[2]), 1.f))
            {
                fn();
            }
        };

    AFTER_MY_CODE
}