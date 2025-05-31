#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkTIFFReader> reader;
    reader->SetFileName("D:/test_data/1.tif");

    vtkNew<vtkImageActor> pActor;
    pActor->GetMapper()->SetInputConnection(reader->GetOutputPort());
    ren->AddActor(pActor);

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("TIFFReader", reader, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}