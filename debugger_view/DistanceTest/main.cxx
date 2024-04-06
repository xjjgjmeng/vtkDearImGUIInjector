#include <ImGuiCommon.h>

vtkns::LogView logView;

namespace
{
    // 点p到直线（lineP0，lineP1）的投影坐标
    std::pair<double, double> projectionFromPoint2Line(const double* lineP0, const double* lineP1, const double* p)
    {
        const auto x0 = p[0];
        const auto y0 = p[1];
        const auto lp0x = lineP0[0];
        const auto lp0y = lineP0[1];
        const auto lp1x = lineP1[0];
        const auto lp1y = lineP1[1];

        if (lp0x == lp1x)
        {
            return { lp0x, p[1] };
        }
        else
        {
            const auto k = (lp0y - lp1y) / (lp0x - lp1x);
            const auto b = lp0y - k * lp0x;
            const auto x1 = (k * (y0 - b) + x0) / (k * k + 1);
            const auto y1 = k * x1 + b;
            return { x1, y1 };
        }
    }

    // 此接口在线的x0==x1且y0==y1的时候会输出nan
    double distancePointToLine(const double origin[3], const double p1[3], const double p2[3])
    {
        //	  origin
        //		/\
		//     /  \   
        //	 b/	   \a	
        //   /		\		  
        // p1--------p2
        //      c
        //通过向量求模长
        //1、根据三点构建三角形，利用余弦定理求出origin与p1构成的线 和 p1,p2构成的线的角度
        //2、利用正弦定理，向量（origin-p1）的模乘以sinA
        //			b2+c2-a2
        //cos(p1) =----------
        //			   2bc
        double b = std::sqrt(std::pow(origin[0] - p1[0], 2) + std::pow(origin[1] - p1[1], 2) + std::pow(origin[2] - p1[2], 2));
        double c = std::sqrt(std::pow(p2[0] - p1[0], 2) + std::pow(p2[1] - p1[1], 2) + std::pow(p2[2] - p1[2], 2));
        double a = std::sqrt(std::pow(origin[0] - p2[0], 2) + std::pow(origin[1] - p2[1], 2) + std::pow(origin[2] - p2[2], 2));
        double value = (b * b + c * c - a * a) / (2 * b * c);
        double A = std::acos(value);
        double distance = std::abs(b * std::sin(A));
        return distance;
    }

    double distancePointToLine2(const double origin[2], const double p1[2], const double p2[2])
    {
        const auto lp0x = p1[0];
        const auto lp0y = p1[1];
        const auto lp1x = p2[0];
        const auto lp1y = p2[1];
        const auto x0 = origin[0];
        const auto y0 = origin[1];
        if (lp0x == lp1x)
        {
            return std::abs(lp0x - x0);
        }
        else
        {
            const auto k = (lp0y - lp1y) / (lp0x - lp1x);
            const auto b = lp0y - k * lp0x;
            return std::abs(k * x0 - y0 + b) / std::sqrt(k * k + 1);
        }
    }
}

namespace
{
    auto getTextActor()
    {
        auto text = vtkSmartPointer<vtkTextActor>::New();
        text->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
        text->GetTextProperty()->SetFontFile("C:/Windows/Fonts/simhei.ttf");
        text->GetTextProperty()->SetColor(0, 1, 0);
        text->GetTextProperty()->SetOpacity(0.8);
        text->GetTextProperty()->SetBackgroundColor(1, 0, 0);
        text->GetTextProperty()->SetBackgroundOpacity(0.5);
        text->GetTextProperty()->SetFontSize(18);
        text->GetTextProperty()->SetJustification(VTK_TEXT_RIGHT);
        //text->GetPositionCoordinate()->SetCoordinateSystemToWorld();
        return text;
    }
}

int main(int argc, char* argv[])
{
    vtkNew<vtkRenderer> ren;
    vtkNew<vtkRenderWindow> renWin;
    renWin->AddRenderer(ren);
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renWin);

    auto lineSource = vtkSmartPointer<vtkLineSource>::New();
    vtkNew<vtkPolyData> pointPolyData;
    vtkIdType pid[1];

    auto updateProjectPoint = [&]
    {
        const double pPoint[3] = { pointPolyData->GetPoint(pid[0])[0], pointPolyData->GetPoint(pid[0])[1] , pointPolyData->GetPoint(pid[0])[2] };
        const auto lp0 = lineSource->GetPoint1();
        const auto lp1 = lineSource->GetPoint2();
        const auto [x, y] = ::projectionFromPoint2Line(lp0, lp1, pPoint);

        static vtkSmartPointer<vtkLineSource> sphereSource;
        static auto text = ::getTextActor();
        static auto projectPointText = ::getTextActor();
        if (!sphereSource.GetPointer())
        {
            sphereSource = vtkSmartPointer<vtkLineSource>::New();
            sphereSource->Update();
            auto mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
            mapper->SetInputConnection(sphereSource->GetOutputPort());
            auto actor = vtkSmartPointer<vtkActor2D>::New();
            actor->SetMapper(mapper);
            actor->GetProperty()->SetColor(1, 1, 0);
            ren->AddActor(actor);
            ren->AddActor(text);
            ren->AddActor(projectPointText);
        }
        const double projectPoint[3] = { x, y, 0 };
        sphereSource->SetPoint1(pPoint);
        sphereSource->SetPoint2(projectPoint);
        projectPointText->SetPosition(const_cast<double*>(projectPoint)); // vtk??!!
        projectPointText->SetInput(fmt::format(u8"投影坐标:{::.2f}\n线段长度:{:.2f}", projectPoint, std::sqrt(vtkMath::Distance2BetweenPoints(lp0, lp1))).c_str());
        text->SetPosition((pPoint[0] + x) / 2, (pPoint[1] + y) / 2);
        text->SetInput(fmt::format(u8"点到直线的投影距离:{:.2f}\n点和投影点的距离:{:.2f}", distancePointToLine2(pPoint, lp0, lp1), std::sqrt(vtkMath::Distance2BetweenPoints(projectPoint, pPoint))).c_str());
    };

    {
        lineSource->SetPoint1(500.0, 500.0, 0.0);  // 起点坐标 
        lineSource->SetPoint2(900.0, 500.0, 0.0);  // 终点坐标
        lineSource->Update();
        auto mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
        mapper->SetInputConnection(lineSource->GetOutputPort());
        auto lineActor = vtkSmartPointer<vtkActor2D>::New();
        lineActor->SetMapper(mapper);
        ren->AddActor(lineActor);
    }

    {
        vtkNew<vtkPoints> points;
        const float p[3] = { 800.0, 600.0, 0.0 };
        vtkNew<vtkCellArray> vertices;
        pid[0] = points->InsertNextPoint(p);
        vertices->InsertNextCell(1, pid);
        pointPolyData->SetPoints(points);
        pointPolyData->SetVerts(vertices);
        vtkNew<vtkPolyDataMapper2D> mapper;
        mapper->SetInputData(pointPolyData);
        vtkNew<vtkActor2D> actor;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(1, 0, 0);
        actor->GetProperty()->SetPointSize(20);
        ren->AddActor(actor);
    }

    updateProjectPoint();

    ::pWindow = renWin;
    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNode("Log"))
            {
                ::logView.Draw();
                ImGui::TreePop();
            }
            
            if (ImGui::DragScalarN("linePoint1", ImGuiDataType_Double, lineSource->GetPoint1(), 2, 0.5f))
            {
                lineSource->Modified();
                updateProjectPoint();
            }
            if (ImGui::DragScalarN("linePoint2", ImGuiDataType_Double, lineSource->GetPoint2(), 2, 0.5f))
            {
                lineSource->Modified();
                updateProjectPoint();
            }
            if (const auto pPoint = pointPolyData->GetPoint(pid[0]); ImGui::DragScalarN("pPoint", ImGuiDataType_Double, pPoint, 2, 0.5f))
            {
                auto newPoints = vtkSmartPointer<vtkPoints>::New();
                newPoints->InsertNextPoint(pPoint);
                pointPolyData->SetPoints(newPoints);
                updateProjectPoint();
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
    //vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    //iren->SetInteractorStyle(vtkSmartPointer<MyStyle>::New());
    iren->EnableRenderOff();
    iren->Start();

    return 0;
}