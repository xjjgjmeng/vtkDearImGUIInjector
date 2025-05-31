#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkJPEGReader> reader;
    reader->SetFileName("./my_resource_dir/Lenna.jpg");
    reader->Update();
    auto img = reader->GetOutput();

    vtkNew<vtkLookupTable> lut;
    lut->SetTableRange(0, 2000);
    lut->SetHueRange(0, 1);
    lut->SetSaturationRange(0, 1); // 灰度图(0,0)
    lut->SetValueRange(0, 1);
    lut->SetRampToLinear();
    lut->Build();

    vtkNew<vtkImageMapToColors> toColors;
    toColors->SetInputData(img);
    toColors->SetLookupTable(lut);

    vtkNew<vtkImageActor> pActor;
    pActor->SetMapper(vtkNew<vtkImageResliceMapper>{});
    pActor->GetMapper()->SetInputConnection(toColors->GetOutputPort());
    ren->AddActor(pActor);

    vtkns::labelWorldZero(ren);

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("ImageMapToColors", toColors, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}