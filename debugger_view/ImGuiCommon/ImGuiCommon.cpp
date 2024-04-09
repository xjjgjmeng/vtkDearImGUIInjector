#include "ImGuiCommon.h"

namespace vtkns
{
    void printWorldPt(vtkns::LogView& logView, vtkRenderer* pRenderer, double disPtX, double disPtY)
    {
        // 0
        {
            double worldPt[4];
            vtkInteractorObserver::ComputeDisplayToWorld(pRenderer, disPtX, disPtY, 0, worldPt);
            logView.Add(fmt::format("worldPt0: {}", worldPt));
        }
        // 1
        {
            vtkNew<vtkCoordinate> coordinate;
            coordinate->SetCoordinateSystemToDisplay();
            coordinate->SetValue(disPtX, disPtY);
            auto worldPt = coordinate->GetComputedWorldValue(pRenderer);
            logView.Add(fmt::format("worldPt1: {}", std::initializer_list{ worldPt[0],worldPt[1], worldPt[2] }));
        }
        // 2
        {
            pRenderer->SetDisplayPoint(disPtX, disPtY, 0);
            pRenderer->DisplayToWorld();
            double worldPt[4];
            pRenderer->GetWorldPoint(worldPt);
            logView.Add(fmt::format("worldPt2: {}", worldPt));
        }
    }
}