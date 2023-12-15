#include <ImGuiCommon.h>
#include <vtkOpenGLActor.h>

ImGuiNs::ExampleAppLog logView;

class MyActor : public vtkOpenGLActor
{
public:
    static MyActor* New();
    vtkTypeMacro(MyActor, vtkOpenGLActor);

    void Pick() override
    {
        ::logView.Add("Pick func");
        __super::Pick();
    }
};
vtkStandardNewMacro(MyActor);

class MyStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyStyle* New();
    vtkTypeMacro(MyStyle, vtkInteractorStyleTrackballCamera);

    void OnLeftButtonDown() override
    {
        int eventPosition[2];
        this->GetInteractor()->GetEventPosition(eventPosition[0], eventPosition[1]);
        ::logView.Add(fmt::format("EventPosition: {}", eventPosition));
        if (const auto renderer = this->GetInteractor()->FindPokedRenderer(eventPosition[0], eventPosition[1]))
        {
            if (auto pPicker = vtkPointPicker::SafeDownCast(this->GetInteractor()->GetPicker()))
            {
                pPicker->Pick(eventPosition[0], eventPosition[1], 0, renderer);
                double pickedPt[3];
                pPicker->GetPickPosition(pickedPt);
                // 在点击的位置放置一个点
                {
                    vtkNew<vtkSphereSource> sphereSource;
                    sphereSource->Update();
                    vtkNew<vtkPolyDataMapper> mapper;
                    mapper->SetInputConnection(sphereSource->GetOutputPort());
                    vtkNew<MyActor> actor;
                    actor->SetMapper(mapper);
                    actor->GetProperty()->SetColor(0, 1, 0);
                    actor->SetPosition(pickedPt);
                    actor->SetScale(0.1);
                    renderer->AddActor(actor);
                }
            }
            else if (auto pPicker = vtkPropPicker::SafeDownCast(this->GetInteractor()->GetPicker()))
            {
                static vtkActor* lastPickedActor = nullptr;
                static vtkNew<vtkProperty> lastPickedProperty;
                pPicker->Pick(eventPosition[0], eventPosition[1], 0, renderer);
                if (lastPickedActor)
                {
                    lastPickedActor->GetProperty()->DeepCopy(lastPickedProperty);
                }

                if (lastPickedActor = pPicker->GetActor())
                {
                    lastPickedProperty->DeepCopy(lastPickedActor->GetProperty());
                    lastPickedActor->GetProperty()->SetColor(1, 0, 0);
                    lastPickedActor->GetProperty()->SetDiffuse(1.0);
                    lastPickedActor->GetProperty()->SetSpecular(0);
                }
            }
        }

        __super::OnLeftButtonDown();
    }

    void OnLeftButtonUp() override
    {
        __super::OnLeftButtonUp();
    }

    void OnMouseMove() override
    {
        __super::OnMouseMove();
    }
};
vtkStandardNewMacro(MyStyle);

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkCubeSource> cubeSource;
    vtkNew<vtkPolyDataMapper> cubeMapper;
    cubeMapper->SetInputConnection(cubeSource->GetOutputPort());
    vtkNew<vtkActor> cubeActor;
    cubeActor->SetMapper(cubeMapper);
    cubeActor->GetProperty()->SetColor(
        colors->GetColor4d("MediumSeaGreen").GetData());
    ren->AddActor(cubeActor);

    auto myStyle = vtkSmartPointer<MyStyle>::New();
    iren->SetInteractorStyle(myStyle);

    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNodeEx("Log", ImGuiTreeNodeFlags_DefaultOpen))
            {
                logView.Draw();
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Camera"))
            {
                ImGuiNs::vtkObjSetup(ren->GetActiveCamera());
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Cube"))
            {
                ImGuiNs::vtkObjSetup(cubeActor);
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Picker", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (const auto v = vtkPointPicker::SafeDownCast(iren->GetPicker()); ImGui::RadioButton("Point", v))
                {
                    if (!v) // 防止多次进入
                    {
                        iren->SetPicker(vtkSmartPointer<vtkPointPicker>::New());
                    }
                }
                ImGui::SameLine();
                if (const auto v = vtkCellPicker::SafeDownCast(iren->GetPicker()); ImGui::RadioButton("Cell", v))
                {
                    if (!v)
                    {
                        iren->SetPicker(vtkSmartPointer<vtkCellPicker>::New());
                    }
                }
                ImGui::SameLine();
                if (const auto v = vtkPropPicker::SafeDownCast(iren->GetPicker()); ImGui::RadioButton("Prop", v))
                {
                    if (!v)
                    {
                        iren->SetPicker(vtkSmartPointer<vtkPropPicker>::New());
                    }
                }
                ImGui::TreePop();
            }
        };

    // Start rendering app
    ren->SetBackground(0.2, 0.3, 0.4);
    renWin->Render(); // 非常重要！！

    /// Change to your code begins here. ///
    // Initialize an overlay with DearImgui elements.
    vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
    // 💉 the overlay.
    dearImGuiOverlay->Inject(iren);
    // These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
    ImguiVtkNs::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    ImguiVtkNs::DrawUI(dearImGuiOverlay);
    /// Change to your code ends here. ///

    vtkNew<vtkCameraOrientationWidget> camManipulator;
    camManipulator->SetParentRenderer(ren);
    camManipulator->On();
    auto rep = vtkCameraOrientationRepresentation::SafeDownCast(camManipulator->GetRepresentation());
    rep->AnchorToLowerRight();

    // Start event loop
#if 0
    renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
// 获取窗口句柄
    HWND hwnd = ::FindWindow(NULL, renWin->GetWindowName());
    // 最大化窗口
    ::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}