#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();

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

    auto pVolume = vtkns::genVR(ren, img, false, true);
    vtkns::genImgOutline(ren, img, false);
    vtkns::labelWorldZero(ren);

    ::imgui_render_callback = [&]
        {
            static bool showOutputImg = false;
            if (bool v = pVolume->GetVisibility(); ImGui::Checkbox("VRVisibility", &v)) pVolume->SetVisibility(v);
            ImGui::SameLine();
            ImGui::Checkbox("ShowOutputImg", &showOutputImg);
            vtkns::vtkObjSetup("SlicePlane", pActor->GetMapper()->GetSlicePlane(), ImGuiTreeNodeFlags_DefaultOpen);
            // vtkns::vtkObjSetup("lut", lut, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("ImageMapToColors", toColors, ImGuiTreeNodeFlags_DefaultOpen);

            if (showOutputImg)
            {
                ImGui::Begin("666");
                vtkns::vtkObjSetup("OutputImg", toColors->GetOutput(), ImGuiTreeNodeFlags_DefaultOpen); // 打开此窗口后，调节rangge会变慢
                ImGui::End();
            }
        };

    AFTER_MY_CODE
}