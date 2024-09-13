#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    auto sliceimg = vtkns::getSliceData();
    vtkns::labelWorldZero(ren);

    vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);

    {
        vtkNew<vtkGPUVolumeRayCastMapper> mapper;
        mapper->SetInputData(img);
        vtkNew<vtkVolume> volume;
        volume->SetMapper(mapper);
        volume->SetPosition(0, 200, 0);
        ren->AddActor(volume);
        ::setupDefaultVolumeProperty(volume);
    }
    vtkNew<vtkImageFlip> volumeFlip;
    {
        volumeFlip->SetInputData(img);
        vtkNew<vtkGPUVolumeRayCastMapper> mapper;
        mapper->SetInputConnection(volumeFlip->GetOutputPort());
        vtkNew<vtkVolume> volume;
        volume->SetMapper(mapper);
        ren->AddActor(volume);
        ::setupDefaultVolumeProperty(volume);
    }

    {
        vtkNew<vtkImageActor> slice;
        slice->GetMapper()->SetInputData(sliceimg);
        slice->SetPosition(200, 200, 0);
        ren->AddActor(slice);
    }
    vtkNew<vtkImageFlip> sliceFlip;
    {
        sliceFlip->SetInputData(sliceimg);
        vtkNew<vtkImageActor> slice;
        slice->GetMapper()->SetInputConnection(sliceFlip->GetOutputPort());
        slice->SetPosition(200, 0, 0);
        ren->AddActor(slice);
    }
    
    ::imgui_render_callback = [&]
        {
            // vtkns::vtkObjSetup("OriginalImageData", img);
            vtkns::vtkObjSetup("VolumeFlip", volumeFlip, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("SliceFlip", sliceFlip, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}