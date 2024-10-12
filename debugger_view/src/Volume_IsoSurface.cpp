#include <ImGuiCommon.h>

int main()
{
	BEFORE_MY_CODE
	vtkns::labelWorldZero(ren);
	auto img = vtkns::getVRData();

	auto volume = vtkns::genVR(ren, img, false, false);

    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("volume", volume, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}