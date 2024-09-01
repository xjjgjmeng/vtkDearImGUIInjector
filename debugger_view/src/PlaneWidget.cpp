#include <ImGuiCommon.h>

class MyCallback : public vtkCallbackCommand
{
public:
    MyCallback() = default;

    static MyCallback* New()
    {
        return new MyCallback;
    }
    void Execute(vtkObject* caller, unsigned long, void*) override
    {
        vtkns::log("123");
    }
};

int main()
{
    BEFORE_MY_CODE
    vtkns::labelWorldZero(ren, false);

    vtkNew<vtkPlaneWidget> w;
    w->SetOrigin(0,0,10);
    w->SetPoint1(0, 0, 0);
    w->SetPoint2(10, 10, 10);
    w->SetInteractor(rwi);
    w->AddObserver(vtkCommand::InteractionEvent, vtkNew<MyCallback>{});
    w->On();

    ::imgui_render_callback = [&]
    {
        vtkns::vtkObjSetup("PlaneWidget", w, ImGuiTreeNodeFlags_DefaultOpen);
    };

    AFTER_MY_CODE
}