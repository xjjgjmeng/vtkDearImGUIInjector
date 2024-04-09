#include <ImGuiCommon.h>
#include <PolyDataHelper.h>

// 比vtkBoundedPlanePointPlacer更容易获取vtkImageData的空间信息
// 但是不能处理点击到Image外面的情况

class MyCameraStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static MyCameraStyle* New();
    vtkTypeMacro(MyCameraStyle, vtkInteractorStyleTrackballCamera);

    void OnMouseMove() override
    {
        int eventPt[2];
        this->GetInteractor()->GetEventPosition(eventPt);
        ::getLogView()->Add(fmt::format("EventPosition: {}", eventPt));
        if (const auto renderer = this->GetInteractor()->FindPokedRenderer(eventPt[0], eventPt[1]))
        {
            vtkns::printWorldPt(*::getLogView(), renderer, eventPt[0], eventPt[1]);

            this->GetInteractor()->GetPicker()->Pick(eventPt[0], eventPt[1], 0, renderer);
            vtkns::Pt_t pickedPt;
            this->GetInteractor()->GetPicker()->GetPickPosition(pickedPt.data()); // 获取pick的世界坐标
            ::getLogView()->Add(fmt::format("pickedPt: {}", pickedPt));

            if (m_b)
            {
                vtkNew<vtkSphereSource> pointSource;
                pointSource->SetCenter(pickedPt.data());
                pointSource->SetRadius(5.0);
                pointSource->Update();

                vtkNew<vtkPolyDataMapper> mapper;
                mapper->SetInputConnection(pointSource->GetOutputPort());

                m_actor->SetMapper(mapper);
                m_actor->GetProperty()->SetColor(1, 0, 0);
                m_actor->GetProperty()->SetPointSize(5);
                renderer->AddActor(m_actor);
            }

            if (auto pCell = vtkCellPicker::SafeDownCast(this->GetInteractor()->GetPicker()); m_bNormal && pCell)
            {
                double normal[3];
                pCell->GetPickNormal(normal);
                vtkMath::Normalize(normal);
                const auto len = 30;
                //auto pt2 = pickedPt;
                //pt2[0] += len * normal[0];
                //pt2[1] += len * normal[1];
                //pt2[2] += len * normal[2];
                auto pt2 = vtkns::advance(pickedPt, normal, len);
                ::getLogView()->Add(fmt::format("len: {}", std::sqrt(vtkMath::Distance2BetweenPoints(pickedPt.data(), pt2.data()))));
                this->m_normal->GetProperty()->SetColor(1, 0, 0);
                this->m_normal->GetProperty()->SetLineWidth(3);
                this->m_normalPt->GetProperty()->SetColor(0, 1, 0);
                this->m_normalPt->GetProperty()->SetPointSize(8);
                vtkns::makeLines(vtkns::Pts_t{ pickedPt, pt2 }, this->m_normal.Get());
                vtkns::makePoints(vtkns::Pts_t{ pt2 }, this->m_normalPt.Get());
                renderer->AddActor(this->m_normal);
                renderer->AddActor(this->m_normalPt);
            }
        }

        __super::OnMouseMove();
    }

    void OnLeftButtonDown() override
    {
        if (this->GetInteractor()->GetControlKey())
        {
            m_bNormal = !m_bNormal;
        }
        else if (this->GetInteractor()->GetShiftKey())
        {
            m_b = !m_b;
        }
        __super::OnLeftButtonDown();
    }

    vtkNew<vtkActor> m_actor;
    vtkNew<vtkActor> m_normal;
    vtkNew<vtkActor> m_normalPt;
    bool m_b = false;
    bool m_bNormal = false;
};
vtkStandardNewMacro(MyCameraStyle);

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    // 设置picker
    {
        auto pPicker = vtkSmartPointer<vtkCellPicker>::New();
        pPicker->SetTolerance(0.0005);
        pPicker->SetPickFromList(1);
        iren->SetPicker(pPicker);
    }

    {
        vtkNew<vtkDICOMImageReader> reader;
        reader->SetFileName(vtkns::getDicomFile());
        reader->Update();

        vtkNew<vtkImageActor> pActor;
        pActor->SetInputData(reader->GetOutput());
        ren->AddActor(pActor);
        iren->GetPicker()->AddPickList(pActor);
    }

    {
        vtkNew<vtkDICOMImageReader> reader;
        reader->SetFileName(vtkns::getDicomFile());
        reader->Update();

        vtkNew<vtkImageActor> pActor;
        pActor->SetInputData(reader->GetOutput());
        pActor->SetScale(0.5);
        //pActor->RotateZ(45);
        pActor->SetPosition(100, 100, 100);
        ren->AddActor(pActor);
        iren->GetPicker()->AddPickList(pActor);
    }

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName(vtkns::getDicomDir());
    reader->Update();

    {
        vtkNew<vtkImageSlice> pImgSlice;
        vtkNew<vtkImageSliceMapper> pMapper;
        pMapper->SetInputData(reader->GetOutput());
        pMapper->SetOrientationToZ();
        pMapper->SetSliceNumber(200);
        
        pImgSlice->SetMapper(pMapper);
        ren->AddActor(pImgSlice);
        iren->GetPicker()->AddPickList(pImgSlice);
    }

    {
        vtkNew<vtkImageActor> pActor;
        vtkNew<vtkImageResliceMapper> pMapper;
        // 设置vtkImageResliceMapper。默认的是vtkImageSliceMapper
        {
            pMapper->SetInputData(reader->GetOutput());
            pMapper->SliceFacesCameraOff();
            pMapper->SliceAtFocalPointOff();
            {
                auto pPlane = pMapper->GetSlicePlane();
                pPlane->SetOrigin(reader->GetOutput()->GetCenter());
                pPlane->SetNormal(1, 1, 1);
            }
            pMapper->UpdateInformation();
            pMapper->Modified();
            pActor->SetMapper(pMapper);
        }
        ren->AddActor(pActor);
        iren->GetPicker()->AddPickList(pActor);
    }

    iren->SetInteractorStyle(vtkNew<MyCameraStyle>{});

    //::showLogView = true;
    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            vtkns::vtkObjSetup("CurrentPicker", iren->GetPicker(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::Begin("PickedActor");
            vtkns::vtkObjSetup("ActorInfo", vtkAbstractPropPicker::SafeDownCast(iren->GetPicker())->GetViewProp(), ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::End();
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