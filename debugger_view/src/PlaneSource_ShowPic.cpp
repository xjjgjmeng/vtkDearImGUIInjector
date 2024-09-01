#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkJPEGReader> reader;
    reader->SetFileName("./my_resource_dir/Lenna.jpg");
    vtkNew<vtkTexture> texture;
    texture->SetInputConnection(reader->GetOutputPort());
    texture->InterpolateOn();
    vtkNew<vtkPlaneSource> plane;
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(plane->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->SetTexture(texture);
    ren->AddActor(actor);

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("PlaneSource", plane, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("ImageActor", actor, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}