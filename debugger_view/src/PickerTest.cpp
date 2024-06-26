﻿#include <ImGuiCommon.h>
#include <vtkOpenGLActor.h>

// https://vtk.org/pipermail/vtkusers/2004-May/024119.html
class MyActor : public vtkOpenGLActor
{
public:
    static MyActor* New();
    vtkTypeMacro(MyActor, vtkOpenGLActor);

    void Pick() override
    {
        ::logger()->Add("Pick func");
        __super::Pick();
    }
};
vtkStandardNewMacro(MyActor);

class MyCameraStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyCameraStyle* New();
    vtkTypeMacro(MyCameraStyle, vtkInteractorStyleTrackballCamera);

    void OnLeftButtonDown() override
    {
        int eventPt[2];
        this->GetInteractor()->GetEventPosition(eventPt);
        ::logger()->Add(fmt::format("EventPosition: {}", eventPt));
        if (const auto renderer = this->GetInteractor()->FindPokedRenderer(eventPt[0], eventPt[1]))
        {
            vtkns::printWorldPt(*::logger(), renderer, eventPt[0], eventPt[1]);

            this->GetInteractor()->GetPicker()->Pick(eventPt[0], eventPt[1], 0, renderer);
            double pickedPt[3];
            this->GetInteractor()->GetPicker()->GetPickPosition(pickedPt); // 获取pick的世界坐标
            ::logger()->Add(fmt::format("pickedPt: {}", pickedPt));
            if (vtkPointPicker::SafeDownCast(this->GetInteractor()->GetPicker()) || vtkWorldPointPicker::SafeDownCast(this->GetInteractor()->GetPicker()))
            {
                // 在点击的位置放置一个点
                vtkNew<vtkSphereSource> sphereSource;
                sphereSource->SetRadius(0.1);
                sphereSource->SetCenter(pickedPt);
                sphereSource->Update();
                vtkNew<vtkPolyDataMapper> mapper;
                mapper->SetInputConnection(sphereSource->GetOutputPort());
                vtkNew<MyActor> actor;
                actor->SetMapper(mapper);
                actor->GetProperty()->SetColor(0, 1, 0);
                renderer->AddActor(actor);
            }
            else if (auto pPicker = vtkPropPicker::SafeDownCast(this->GetInteractor()->GetPicker()))
            {
                static vtkActor* lastPickedActor = nullptr;
                static vtkNew<vtkProperty> lastPickedProperty;
                if (lastPickedActor)
                {
                    lastPickedActor->GetProperty()->DeepCopy(lastPickedProperty);
                }

                if (lastPickedActor = pPicker->GetActor())
                {
                    lastPickedProperty->DeepCopy(lastPickedActor->GetProperty());
                    lastPickedActor->GetProperty()->SetColor(0.5, 0., 0.);
                }
            }
            else if (auto pPicker = vtkCellPicker::SafeDownCast(this->GetInteractor()->GetPicker()))
            {
                //pPicker->SetTolerance(0.0005);
                if (const auto cellId = pPicker->GetCellId(); -1 != cellId && 0)
                {
                    vtkNew<vtkIdTypeArray> ids;
                    ids->SetNumberOfComponents(1);
                    ids->InsertNextValue(cellId);

                    vtkNew<vtkSelectionNode> selectionNode;
                    selectionNode->SetFieldType(vtkSelectionNode::CELL);
                    selectionNode->SetContentType(vtkSelectionNode::INDICES);
                    selectionNode->SetSelectionList(ids);

                    vtkNew<vtkSelection> selection;
                    selection->AddNode(selectionNode);

                    vtkNew<vtkExtractSelection> extractSelection;
                    auto pickedActor = vtkActor::SafeDownCast(pPicker->GetProp3D());
                    extractSelection->SetInputData(0, pickedActor->GetMapper()->GetInput());
                    extractSelection->SetInputData(1, selection);
                    extractSelection->Update();

                    vtkNew<vtkDataSetMapper> selectedMapper;
#if 0
                    selectedMapper->SetInputConnection(extractSelection->GetOutputPort());
#else
                    vtkNew<vtkUnstructuredGrid> selected;
                    selected->ShallowCopy(extractSelection->GetOutput());
                    selectedMapper->SetInputData(selected);
#endif
                    vtkNew<vtkActor> actor;
                    actor->SetMapper(selectedMapper);
                    actor->GetProperty()->EdgeVisibilityOn();
                    actor->GetProperty()->SetEdgeColor(1,1,0);
                    actor->GetProperty()->SetLineWidth(13);
                    actor->SetPosition(pPicker->GetActor()->GetPosition());
                    renderer->AddActor(actor);
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
vtkStandardNewMacro(MyCameraStyle);

class MyActorStyle : public vtkInteractorStyleTrackballActor
{
public:
    static MyActorStyle* New();
    vtkTypeMacro(MyActorStyle, vtkInteractorStyleTrackballActor);

    void OnLeftButtonDown() override
    {
        __super::OnLeftButtonDown(); // 此函数会调用vtkInteractorStyleTrackballActor::FindPickedActor，更新InteractionPicker变量
        ::logger()->Add(fmt::format("InteractionProp: {}", reinterpret_cast<void*>(this->InteractionProp)));
        ::logger()->Add(fmt::format("GetViewProp: {}", reinterpret_cast<void*>(this->InteractionPicker->GetViewProp())));
    }
};
vtkStandardNewMacro(MyActorStyle);

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkCubeSource> cubeSource;
    cubeSource->SetBounds(-10, 10, -10, 10, -10, 10);
    vtkNew<vtkPolyDataMapper> cubeMapper;
    cubeMapper->SetInputConnection(cubeSource->GetOutputPort());
    vtkNew<vtkActor> cubeActor;
    cubeActor->SetMapper(cubeMapper);
    cubeActor->GetProperty()->SetColor(
        colors->GetColor4d("MediumSeaGreen").GetData());
    ren->AddActor(cubeActor);

    {
        vtkNew<vtkLineSource> pSource;
        pSource->SetPoint1(-20, -20, -20);
        pSource->SetPoint2(20, 20, 20);
        pSource->Update();

        vtkNew<vtkPolyDataMapper2D> pMapper;
        {
            vtkNew<vtkCoordinate> pCoordinate;
            pCoordinate->SetCoordinateSystemToWorld();
            pMapper->SetTransformCoordinate(pCoordinate);
        }
        pMapper->SetInputData(pSource->GetOutput());

        vtkNew<vtkActor2D> pActor;
        pActor->SetMapper(pMapper);
        ren->AddActor(pActor);
    }

    {
        vtkNew<vtkDICOMImageReader> reader;
        reader->SetFileName(vtkns::getDicomFile());
        reader->Update();

        vtkNew<vtkImageActor> pActor;
        pActor->SetInputData(reader->GetOutput());
        ren->AddActor(pActor);
    }

    auto myStyle = vtkSmartPointer<MyCameraStyle>::New();
    iren->SetInteractorStyle(myStyle);

    ::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("Cube", cubeActor);
            if (ImGui::TreeNodeEx("Picker", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const auto pPicker = iren->GetPicker(); // pPicker并不是一直有效的，比如SetPicker设置了新的picker后此变量所指之物就失效了
                vtkNotUsed(pPicker);
                if (const auto v = vtkPointPicker::SafeDownCast(iren->GetPicker()); ImGui::RadioButton("Point", v))
                {
                    if (!v) // 防止多次进入。如果以前不是这个picker才设置这个picker
                    {
                        iren->SetPicker(vtkSmartPointer<vtkPointPicker>::New());
                    }
                }
                ImGui::SameLine();
                // It works for all Prop3Ds ??
                if (const auto v = vtkCellPicker::SafeDownCast(iren->GetPicker()); ImGui::RadioButton("Cell", v))
                {
                    if (!v)
                    {
                        auto pPicker = vtkSmartPointer<vtkCellPicker>::New();
                        pPicker->SetTolerance(0.0005);
                        iren->SetPicker(pPicker);
                    }
                }
                ImGui::SameLine();
                // vtkPropPicker使用硬件拾取的策略来确定所拾取的vtkProp对象，包括拾取点的世界坐标。
                // 它通常比vtkAbstractPropPicker的其它子类速度要快，但它不能返回所拾取对象的详细信息
                if (const auto v = vtkPropPicker::SafeDownCast(iren->GetPicker()); ImGui::RadioButton("Prop", v))
                {
                    if (!v)
                    {
                        iren->SetPicker(vtkSmartPointer<vtkPropPicker>::New());
                    }
                }
                ImGui::SameLine();
                if (const auto v = vtkWorldPointPicker::SafeDownCast(iren->GetPicker()); ImGui::RadioButton("WorldPoint", v))
                {
                    if (!v)
                    {
                        iren->SetPicker(vtkSmartPointer<vtkWorldPointPicker>::New());
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Style", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (const auto v = MyCameraStyle::SafeDownCast(iren->GetInteractorStyle()); ImGui::RadioButton("Camera", v))
                {
                    if (!v)
                    {
                        iren->SetInteractorStyle(vtkNew<MyCameraStyle>{});
                    }
                }
                ImGui::SameLine();
                if (const auto v = MyActorStyle::SafeDownCast(iren->GetInteractorStyle()); ImGui::RadioButton("Actor", v))
                {
                    if (!v)
                    {
                        iren->SetInteractorStyle(vtkNew<MyActorStyle>{});
                    }
                }
                //ImGui::SameLine();
                //if (const auto v = DrawFreeLineStyle::SafeDownCast(iren->GetInteractorStyle()); ImGui::RadioButton("DrawFreeLineStyle", v))
                //{
                //    if (!v)
                //    {
                //        iren->SetInteractorStyle(vtkSmartPointer<DrawFreeLineStyle>::New());
                //    }
                //}
                ImGui::TreePop();
            }
            vtkns::vtkObjSetup("CurrentPicker", iren->GetPicker(), ImGuiTreeNodeFlags_DefaultOpen);
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
    vtkns::SetupUI(dearImGuiOverlay);
    // You can draw custom user interface elements using ImGui:: namespace.
    vtkns::DrawUI(dearImGuiOverlay);
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