#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    vtkNew<vtkLookupTable> lut;
    lut->SetNumberOfColors(100);		// 指定颜色查找表中有多少种颜色
    lut->SetHueRange(0.00, 0.6667);	//蓝到红渐变
    lut->SetTableRange(0, 666);
    lut->Build();

    vtkNew<vtkScalarBarActor> bar;
    bar->SetTitle("MyTitle");
    bar->SetLookupTable(lut);
    ren->AddActor2D(bar);

    ren->ResetCamera();

    ::pWindow = rw;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("lut", lut, ImGuiTreeNodeFlags_DefaultOpen);
            vtkns::vtkObjSetup("bar", bar, ImGuiTreeNodeFlags_DefaultOpen);
        };

    AFTER_MY_CODE
}