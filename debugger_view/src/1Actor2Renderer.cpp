#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);
    auto volume = vtkns::genVR(ren, img, false, false);
    ren->SetViewport(0, 0, 0.5, 1);
    ren->SetBackground(0, 0, 0);

    vtkNew<vtkRenderer> ren2;
    ren2->SetViewport(0.5, 0, 1, 1);
    //ren2->SetBackground(1, 1, 1);
    ren2->AddActor(volume);
    rw->AddRenderer(ren2);

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("Volume", volume, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}