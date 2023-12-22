#include "DrawFreeLineStyle.h"

#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>

vtkStandardNewMacro(DrawFreeLineStyle);

void DrawFreeLineStyle::OnLeftButtonDown()
{
    this->m_pressed = true;
}

void DrawFreeLineStyle::OnLeftButtonUp()
{
    this->m_pressed = false;
    this->m_currLineSource = nullptr;
}

void DrawFreeLineStyle::OnMouseMove()
{
    if (this->m_pressed)
    {
        int currPt[2];
        this->GetInteractor()->GetEventPosition(currPt);
        if (const auto pRenderer = this->GetInteractor()->FindPokedRenderer(currPt[0], currPt[1]))
        {
            double worldPt[4];
            vtkInteractorObserver::ComputeDisplayToWorld(pRenderer, currPt[0], currPt[1], 0, worldPt);
            if (!this->m_currLineSource)
            {
                auto lineSource = vtkSmartPointer<vtkLineSource>::New();
                this->m_currLineSource = lineSource; // 如果在上一句直接赋值会崩溃
                {
                    vtkNew<vtkPoints> newPoints;
                    newPoints->InsertNextPoint(worldPt); // note: 前两个点是相同的
                    lineSource->SetPoints(newPoints);
                }
                vtkNew<vtkPolyDataMapper> mapper;
                mapper->SetInputConnection(lineSource->GetOutputPort());
                vtkNew<vtkActor> actor;
                actor->GetProperty()->SetColor(1, 0, 0);
                actor->GetProperty()->SetLineWidth(6.f);
                actor->SetMapper(mapper);
                pRenderer->AddActor(actor);
            }
            this->m_currLineSource->GetPoints()->InsertNextPoint(worldPt);
            this->m_currLineSource->Modified();
            this->GetInteractor()->GetRenderWindow()->Render();
        }
    }
}