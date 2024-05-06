#include <ImGuiCommon.h>

vtkns::LogView logView;

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    vtkNew<vtkPoints> pPoints;
    pPoints->InsertNextPoint(0, 0, 0);
    pPoints->InsertNextPoint(50, 50, 0);
    pPoints->InsertNextPoint(90, 30, 0);

    vtkNew<vtkCellArray> pCellArray;
#if 0
    for (int i = 0; i < pPoints->GetNumberOfPoints()-1; ++i)
    {
        vtkNew<vtkLine> line;
        line->GetPointIds()->SetId(0, i);
        line->GetPointIds()->SetId(1, i + 1);
        pCellArray->InsertNextCell(line);
    }
#else
    vtkNew<vtkPolyLine> pPolyLine;
    pPolyLine->GetPointIds()->SetNumberOfIds(pPoints->GetNumberOfPoints());
    for (int i = 0; i < pPoints->GetNumberOfPoints(); ++i)
    {
        pPolyLine->GetPointIds()->SetId(i, i);
    }
    pCellArray->InsertNextCell(pPolyLine);
#endif

    vtkNew<vtkPolyData> pPolyData;
    pPolyData->SetPoints(pPoints);
    pPolyData->SetLines(pCellArray);

    vtkNew<vtkPolyDataMapper> pMapper;
    pMapper->SetInputData(pPolyData);

    vtkNew<vtkActor> pActor;
    pActor->SetMapper(pMapper);
    pActor->GetProperty()->SetVertexVisibility(true);
    pActor->GetProperty()->SetEdgeVisibility(true);
    pActor->GetProperty()->SetPointSize(33);
    pActor->GetProperty()->SetVertexColor(1,0,0);
    pActor->GetProperty()->SetColor(1,1,0);
    pActor->GetProperty()->SetRenderPointsAsSpheres(true);
    ren->AddActor(pActor);

    vtkNew<vtkKdTreePointLocator> kDTree;
    kDTree->SetDataSet(pPolyData);
    kDTree->BuildLocator();

    vtkNew<vtkCallbackCommand> pCmd;
    pCmd->SetClientData(kDTree);
    pCmd->SetCallback([](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
        {
            auto pI = vtkRenderWindowInteractor::SafeDownCast(caller);
            auto pKdTree = reinterpret_cast<vtkKdTreePointLocator*>(clientdata);
            auto pDataSet = pKdTree->GetDataSet();
            int eventPt[2];
            pI->GetEventPosition(eventPt);
            auto pR = pI->FindPokedRenderer(eventPt[0], eventPt[1]);
            double worldPt[4];
            vtkInteractorObserver::ComputeDisplayToWorld(pR, eventPt[0], eventPt[1], 0, worldPt);
            const auto id = pKdTree->FindClosestPoint(worldPt);
            double pPt[3];
            pDataSet->GetPoint(id, pPt);

            vtkNew<vtkSphereSource> pSphereSource;
            pSphereSource->SetRadius(5);
            pSphereSource->Update();
            vtkNew<vtkPolyDataMapper> pMapper;
            pMapper->SetInputData(pSphereSource->GetOutput());
            static vtkNew<vtkActor> pActor;
            pActor->SetMapper(pMapper);
            pActor->SetPosition(pPt);
            pR->AddActor(pActor);
        });
    iren->AddObserver(vtkCommand::LeftButtonPressEvent, pCmd);

    ren->SetBackground(0, 0, 0);
    ren->ResetCamera();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            vtkns::vtkObjSetup("Actor", pActor);
			if (ImGui::Button("Resample"))
			{
				vtkNew<vtkPoints> resampledPoints;
				ResamplePoints(pPoints, resampledPoints, 10.0, 1);
				auto resampledLinePoly = ::convertPointsToPolyline(resampledPoints);
				pMapper->SetInputData(resampledLinePoly);
                kDTree->SetDataSet(resampledLinePoly);
			}
			if (ImGui::Button("Distance"))
			{
				static vtkSmartPointer<vtkDistanceWidget> distanceWidget;
				distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
				{
					distanceWidget->SetInteractor(iren);
					distanceWidget->CreateDefaultRepresentation();
					vtkDistanceRepresentation::SafeDownCast(distanceWidget->GetRepresentation())->SetLabelFormat("%-#6.3g mm");
					renWin->Render();
					distanceWidget->On();
				}
			}
        };

    // Start rendering app
    ren->SetBackground(0., 0., 0.);
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
    vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballActor();
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}