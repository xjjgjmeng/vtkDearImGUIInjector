#include <ImGuiCommon.h>

vtkNew<vtkBoundedPlanePointPlacer> placer;
double pt[3];
bool isValid;

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyStyle* New();
    MyStyle()
    {
        ::placer->SetProjectionNormalToOblique();
        {
            vtkNew<vtkPlane> plane;
            plane->SetOrigin(0, 0, 0);
            plane->SetNormal(0, 0, 1);
            ::placer->SetObliquePlane(plane);
        }
    }

    void OnLeftButtonDown() override
    {
        int eventPt[2];
        this->Interactor->GetEventPosition(eventPt);
        const auto ren = this->Interactor->FindPokedRenderer(eventPt[0], eventPt[1]);

        double eventPt_[] = { eventPt[0], eventPt[1] };
        ::isValid = ::placer->ComputeWorldPosition(ren, eventPt_, pt, TmpArr<double[9]>);
        vtkns::addPt(ren, { ::pt[0], pt[1], pt[2] });

        __super::OnLeftButtonDown();
    }
};
vtkStandardNewMacro(MyStyle);

int main()
{
    BEFORE_MY_CODE
    rwi->SetInteractorStyle(vtkNew<MyStyle>{});
    vtkns::labelWorldZero(ren);

    ::imgui_render_callback = [&]
    {
        vtkns::ImGuiText(u8"最新有效点: {::.2f}", ::pt); ImGui::SameLine();
        ImGui::TextColored(::isValid?ImVec4{0.f,1.f,0.f,0.5f}:ImVec4{1.f,0.f,0.f,0.5f}, ::isValid?"Valid":"Invalid");
        vtkns::vtkObjSetup("Placer", ::placer, ImGuiTreeNodeFlags_DefaultOpen);
    };

    AFTER_MY_CODE
}