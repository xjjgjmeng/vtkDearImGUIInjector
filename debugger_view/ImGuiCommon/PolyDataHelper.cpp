#include "PolyDataHelper.h"

#include <vtkLine.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkCoordinate.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkTextProperty.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>

namespace vtkns
{
    vtkSmartPointer<vtkPolyData> makeLines(const std::list<Pt_t>& pts)
    {
        vtkNew<vtkPoints> pPoints;
        vtkNew<vtkCellArray> pLines;

        // points
        for (const auto& i : pts)
        {
            pPoints->InsertNextPoint(i.data());
        }

        // lines
        for (int i = 0; i < int(std::size(pts)) - 1; ++i)
        {
            vtkNew<vtkLine> pLine;
            pLine->GetPointIds()->SetId(0, i);
            pLine->GetPointIds()->SetId(1, i + 1);
            pLines->InsertNextCell(pLine);
        }

        vtkNew<vtkPolyData> pPolyData;
        pPolyData->SetPoints(pPoints);
        pPolyData->SetLines(pLines);

        return pPolyData;
    }

    void makeLines(const std::list<Pt_t>& pts, vtkActor2D* pActor)
    {
        vtkNew<vtkPolyDataMapper2D> pMapper;
        pMapper->SetInputData(vtkns::makeLines(pts));
        {
            vtkNew<vtkCoordinate> pCoordinate;
            pCoordinate->SetCoordinateSystemToWorld();
            pMapper->SetTransformCoordinate(pCoordinate);
        }
        pActor->SetMapper(pMapper);
    }

    void makeLines(const std::list<Pt_t>& pts, vtkActor* pActor)
    {
        vtkNew<vtkPolyDataMapper> pMapper;
        pMapper->SetInputData(vtkns::makeLines(pts));
        pActor->SetMapper(pMapper);
    }

    vtkSmartPointer<vtkPolyData> makePoints(const std::list<Pt_t>& pts)
    {
        vtkNew<vtkPoints> pPoints;
        vtkNew<vtkCellArray> pVerts;

        for (int i = 0; i < int(std::size(pts)); ++i)
        {
            pPoints->InsertNextPoint(std::next(std::begin(pts),i)->data());

            vtkIdType pt[1] = { i };
            pVerts->InsertNextCell(1, pt);
        }

        vtkNew<vtkPolyData> pPolyData;
        pPolyData->SetPoints(pPoints);
        pPolyData->SetVerts(pVerts);

        return pPolyData;
    }

    void makePoints(const std::list<Pt_t>& pts, vtkActor2D* pActor)
    {
        vtkNew<vtkPolyDataMapper2D> pMapper;
        pMapper->SetInputData(vtkns::makePoints(pts));
        {
            vtkNew<vtkCoordinate> pCoordinate;
            pCoordinate->SetCoordinateSystemToWorld();
            pMapper->SetTransformCoordinate(pCoordinate);
        }

        pActor->SetMapper(pMapper);
    }

    void makePoints(const std::list<Pt_t>& pts, vtkActor* pActor)
    {
        vtkNew<vtkPolyDataMapper> pMapper;
        pMapper->SetInputData(vtkns::makePoints(pts));
        pActor->SetMapper(pMapper);
    }

    void addPt(vtkRenderer* ren, const vtkns::Pt_t& pt)
    {
        vtkNew<vtkActor> actor;
        vtkns::makePoints({ pt }, actor);
        ren->AddViewProp(actor);
        actor->GetProperty()->SetPointSize(13);
        actor->GetProperty()->SetRenderPointsAsSpheres(1);
        ren->GetRenderWindow()->Render();
    }

    Pt_t advance(const Pt_t& pt, const double direction[3], const double len)
    {
        auto retval = pt;
        double directionNormal[] = {direction[0], direction[1], direction[2]};
        vtkMath::Normalize(directionNormal);

        retval[0] += len * directionNormal[0];
        retval[1] += len * directionNormal[1];
        retval[2] += len * directionNormal[2];

        return retval;
    }

    vtkSmartPointer<vtkTextActor> genTextActor()
    {
        auto text = vtkSmartPointer<vtkTextActor>::New();
        text->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
        text->GetTextProperty()->SetFontFile("C:/Windows/Fonts/simhei.ttf");
        text->GetTextProperty()->SetColor(vtkns::TextNormalColor);
        text->GetTextProperty()->SetOpacity(1.);
        text->GetTextProperty()->SetBackgroundColor(1, 1, 1);
        text->GetTextProperty()->SetBackgroundOpacity(0.);
        text->GetTextProperty()->SetFontSize(13);
        //text->GetTextProperty()->SetJustification(VTK_TEXT_CENTERED);
        text->GetPositionCoordinate()->SetCoordinateSystemToWorld();
        return text;
    }

    std::string doubleToString(const double v)
    {
        char buf[256]{};
        std::snprintf(buf, sizeof(buf), "%.2f", v);
        return buf;
    }
    std::pair<vtkActor*, vtkPolyDataMapper*> renderPolydata(vtkRenderer* ren, vtkAlgorithmOutput* algoOutPort)
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(algoOutPort);
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        ren->AddActor(actor);
        return std::make_pair(actor.Get(), mapper.Get());
    }
}