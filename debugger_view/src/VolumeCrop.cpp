#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE
    auto img = vtkns::getVRData();
    vtkns::labelWorldZero(ren);

    // 将原始的image用线框显示出来
    vtkns::genImgOutline(ren, img, false)->GetProperty()->SetColor(1., 1., 0.);
    auto volume = vtkns::genVR(ren, img, false);

    vtkNew<vtkPlane> planeArr[6];
    vtkNew<vtkPlaneWidget> planeWidgetArr[std::size(planeArr)];

    for (auto i = 0; i < 6; ++i)
    {
        volume->GetMapper()->AddClippingPlane(planeArr[i]);
        auto bounds = img->GetBounds();
        planeWidgetArr[i]->SetOrigin(bounds[0], bounds[2], bounds[5]);
        planeWidgetArr[i]->SetPoint1(bounds[0], bounds[2], bounds[4]);
        planeWidgetArr[i]->SetPoint2(bounds[1], bounds[3], bounds[5]);
        planeWidgetArr[i]->SetInteractor(rwi);
        planeWidgetArr[i]->On();
    }

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("OriginalImageData", img);
        vtkns::vtkObjSetup("Volume", volume, ImGuiTreeNodeFlags_DefaultOpen);
        for (auto i = 0; i < std::size(planeArr); ++i)
        {
            planeWidgetArr[i]->GetPlane(planeArr[i].GetPointer());
        }
        for (auto i = 0; i < std::size(planeArr); ++i)
        {
            vtkns::vtkObjSetup(("PlaneWidget"+std::to_string(i)).c_str(), planeWidgetArr[i]);
        }
    };

    AFTER_MY_CODE
}