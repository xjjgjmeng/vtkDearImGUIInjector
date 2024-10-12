#include <ImGuiCommon.h>

int main()
{
	BEFORE_MY_CODE
	vtkns::labelWorldZero(ren);
	auto img = vtkns::getVRData();

	vtkNew<vtkImageSincInterpolator> interpolator;
	interpolator->SetWindowFunctionToHann();
	interpolator->AntialiasingOn();

	vtkNew<vtkImageResize> resize;
	resize->SetInputData(img);
	resize->InterpolateOn();
	resize->SetInterpolator(interpolator);
	resize->SetResizeMethodToOutputDimensions();
	resize->SetOutputDimensions(img->GetDimensions());
	resize->SetOutputSpacing(img->GetSpacing());
	resize->SetCroppingRegion(img->GetBounds());
	vtkns::genImgOutline(ren, resize->GetOutputPort());
	// vtkns::genImgMesh(ren, resize->GetOutputPort());
	auto volume = vtkns::genVR(ren, resize->GetOutputPort(), false, false);
    ren->AddVolume(volume);

    ::imgui_render_callback = [&]
        {
			vtkns::vtkObjSetupWin("OriginalImageData", img);
			vtkns::vtkObjSetupWin("ResizeOutput", resize->GetOutput());
            vtkns::vtkObjSetup("volume", volume);
			vtkns::vtkObjSetup("resize", resize, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}