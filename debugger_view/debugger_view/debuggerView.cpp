#include "debuggerView.h"



#include "vtkDearImGuiInjector.h"

#include "Common.h"

void init()
{
    ::gData->InteractionPicker = vtkSmartPointer<vtkCellPicker>::New();
    ::gData->InteractionPicker->SetTolerance(0.001);

    ::gData->PropPicker = vtkSmartPointer<vtkPropPicker>::New();
}

void showView(vtkRenderer *renderer)
{
    vtkNew<vtkNamedColors> colors;

    // Sphere
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    ::gData->polyData = sphereSource->GetOutput();

    ::gData->actor = vtkSmartPointer<vtkActor>::New();
    ::gData->actor->SetMapper(mapper);
    ::gData->actor->GetProperty()->SetColor(
        colors->GetColor3d("DarkOliveGreen").GetData());

    // A renderer and render window.
    //vtkNew<vtkRenderer> renderer;
    //vtkNew<vtkRenderWindow> renderWindow;
    //renderWindow->AddRenderer(renderer);
    //renderWindow->SetWindowName("CaptionWidget");

    // An interactor
    //vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    //renderWindowInteractor->SetRenderWindow(renderWindow);

    // Create the widget and its representation.
    ::gData->captionRepresentation = vtkSmartPointer<vtkCaptionRepresentation>::New();
    ::gData->captionRepresentation->GetCaptionActor2D()->SetCaption("Test caption");
    ::gData->captionRepresentation->GetCaptionActor2D()
        ->GetTextActor()
        ->GetTextProperty()
        ->SetFontSize(100);

    double pos[3] = { .5, 0, 0 };
    ::gData->captionRepresentation->SetAnchorPosition(pos);

    ::gData->captionWidget = vtkSmartPointer<vtkCaptionWidget>::New();
    ::gData->captionWidget->SetInteractor(renderer->GetRenderWindow()->GetInteractor());
    ::gData->captionWidget->SetRepresentation(::gData->captionRepresentation);

    // Add the actors to the scene.
    renderer->AddActor(::gData->actor);
    renderer->SetBackground(colors->GetColor3d("Blue").GetData());

    renderer->GetRenderWindow()->Render();

    // Rotate the camera to bring the point the caption is pointing to into view.
    renderer->GetActiveCamera()->Azimuth(90);

    ::gData->captionWidget->On();


    ::gData->lineActor = vtkSmartPointer<vtkActor2D>::New();
    ::gData->lineActor->PickableOn();
    //auto linesource = vtkSmartPointer<vtkLineSource>::New();
    vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(500.0, 500.0, 0.0);  // 起点坐标 
    lineSource->SetPoint2(900.0, 500.0, 0.0);  // 终点坐标
    lineSource->Update();

    // 创建映射器（Mapper）
    vtkSmartPointer<vtkPolyDataMapper2D> mapper1 = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    mapper1->SetInputConnection(lineSource->GetOutputPort());

    // 创建演员（Actor）
    //vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    ::gData->lineActor->SetMapper(mapper1);
    ::gData->lineActor->SetPickable(true);
    renderer->AddActor(::gData->lineActor);


    renderer->ResetCamera();
}

void showView1(vtkDearImGuiInjector* overlay_)
{
    //if (ImGui::CollapsingHeader("657575", ImGuiTreeNodeFlags_DefaultOpen))
    //{
    //    auto rw = overlay_->Interactor->GetRenderWindow();
    //    ImGui::Text("MTime: %ld", rw->GetMTime());
    //    ImGui::Text("Name: %s", rw->GetClassName());
    //    if (ImGui::TreeNode("Capabilities"))
    //    {
    //        ImGui::TextWrapped("OpenGL: %s", rw->ReportCapabilities());
    //        ImGui::TreePop();
    //    }
    //}
    if (ImGui::CollapsingHeader("debugger", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto rw = overlay_->Interactor->GetRenderWindow();
        ImGui::Text("MTime: %ld", rw->GetMTime());
        ImGui::Text("Name: %s", rw->GetClassName());
        if (ImGui::TreeNode("CaptionWidget")) // 如果这里使用了Capabilities，着也会展开vtkRenderWindow的Capabilities
        {
            bool captionOn = gData->captionWidget->GetEnabled();
            if (ImGui::Checkbox("On", &captionOn))
            {
                gData->captionWidget->SetEnabled(captionOn);
            }
            ImGui::SameLine();
            bool bLeader = gData->captionWidget->GetCaptionActor2D()->GetLeader();
            if (ImGui::Checkbox("Leader", &bLeader))
            {
                gData->captionWidget->GetCaptionActor2D()->SetLeader(bLeader);
            }
            ImGui::SameLine();
            bool bBorder = gData->captionWidget->GetCaptionActor2D()->GetBorder();
            if (ImGui::Checkbox("Border", &bBorder))
            {
                gData->captionWidget->GetCaptionActor2D()->SetBorder(bBorder);
            }

            double pos[3];
            gData->captionRepresentation->GetAnchorPosition(pos);
            if (ImGui::DragScalarN("AnchorPosition", ImGuiDataType_Double, pos, 3, 0.001))
            {
                gData->captionRepresentation->SetAnchorPosition(pos);
            }

            int fontSize = gData->captionRepresentation->GetCaptionActor2D()
                ->GetTextActor()
                ->GetTextProperty()
                ->GetFontSize();
            if (ImGui::SliderInt("FontSize", &fontSize, 1, 1000))
            {
                gData->captionRepresentation->GetCaptionActor2D()
                    ->GetTextActor()
                    ->GetTextProperty()
                    ->SetFontSize(fontSize);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("CurrentActor"))
        {
            if (gData->InteractionProp)
            {
                //ImGui::TextWrapped("%s", gData->InteractionProp->GetObjectDescription());
                ImGui::TextWrapped("%s", gData->InteractionProp->GetClassName());
            }
            else
            {
                ImGui::Text("None");
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Log"))
        {
          gData->log.Draw();
          ImGui::TreePop();
        }
    }
}
