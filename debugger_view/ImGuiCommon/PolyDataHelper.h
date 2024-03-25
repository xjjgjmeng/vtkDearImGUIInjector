#pragma once

#include <vector>
#include <array>
#include <list>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkActor2D.h>
#include <vtkActor.h>
#include <vtkTextActor.h>

namespace vtkNs
{
    using Pt_t = std::array<double, 3>;
    using Pts_t = std::list<Pt_t>;

    constexpr double NormalColor[3] = { 1., 0., 0. };
    constexpr double SelectionColor[3] = { 1., 1., 0. };
    constexpr double TextNormalColor[3] = { 1., 1., 1. };
    constexpr double TextSelectionColor[3] = { 1., 1., 0. };

    vtkSmartPointer<vtkPolyData> makeLines(const std::list<Pt_t>& pts); // amazing effect!!
    void makeLines(const std::list<Pt_t>& pts, vtkActor2D* pActor);
    void makeLines(const std::list<Pt_t>& pts, vtkActor* pActor);
    vtkSmartPointer<vtkPolyData> makePoints(const std::list<Pt_t>& pts);
    void makePoints(const std::list<Pt_t>& pts, vtkActor2D* pActor);
    void makePoints(const std::list<Pt_t>& pts, vtkActor* pActor);

    // 获取pt沿着direction方向前进len后到达的点
    Pt_t advance(const Pt_t& pt, const double direction[3], const double len);

    vtkSmartPointer<vtkTextActor> genTextActor();
    std::string doubleToString(const double v);
}