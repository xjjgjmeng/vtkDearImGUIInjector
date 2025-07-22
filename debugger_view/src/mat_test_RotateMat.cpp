#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    {
        vtkNew<vtkAxesActor> axes;
        ren->AddActor(axes);
        axes->SetAxisLabels(false);
        axes->GetXAxisShaftProperty()->SetOpacity(0.8);
        axes->GetYAxisShaftProperty()->SetOpacity(0.8);
        axes->GetZAxisShaftProperty()->SetOpacity(0.8);
        axes->SetNormalizedTipLength(0.0, 0.0, 0.0);
    }

    vtkNew<vtkMatrix4x4> myMat;
    {
        auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
        {
            static vtkNew<vtkAxesActor> axes;
            axes->SetAxisLabels(false);
            reinterpret_cast<vtkRenderer*>(clientdata)->AddActor(axes);
            axes->SetUserMatrix(vtkMatrix4x4::SafeDownCast(caller));
        };
        vtkNew<vtkCallbackCommand> pCC;
        pCC->SetCallback(f);
        pCC->SetClientData(ren);
        myMat->AddObserver(vtkCommand::ModifiedEvent, pCC);

        const double arr[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        myMat->DeepCopy(arr);
    }

    ::imgui_render_callback = [&]
    {
            static bool bPostMultiply = true;
            static bool bRotateAroundCustomline = false;
            static double rotateAxisPtBegin[4]{ 1.,0.,0., 1. };
            static double rotateAxisPtEnd[4]{ 1.,1.,1., 1. };

            if (ImGui::TreeNodeEx("RotateAxis", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragScalarN(u8"起", ImGuiDataType_Double, rotateAxisPtBegin, 3, 0.01f);
                ImGui::DragScalarN(u8"止", ImGuiDataType_Double, rotateAxisPtEnd, 3, 0.01f);

                // 原始旋转轴
                {
                    static vtkNew<vtkLeaderActor2D> rotateAxis;
                    rotateAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
                    rotateAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();
                    rotateAxis->SetArrowStyleToOpen();
                    rotateAxis->SetArrowPlacementToPoint2();
                    rotateAxis->GetProperty()->SetColor(1, 1, 0);
                    rotateAxis->GetProperty()->SetOpacity(0.3);
                    //rotateAxis->SetLabel("OriginalAxis");
                    ren->AddViewProp(rotateAxis);
                    rotateAxis->GetPositionCoordinate()->SetValue(rotateAxisPtBegin);
                    rotateAxis->GetPosition2Coordinate()->SetValue(rotateAxisPtEnd);
                }
                // 原始旋转轴平移至世界中心
                {
                    static vtkNew<vtkActor> myLine;
                    myLine->GetProperty()->SetOpacity(0.5);
                    myLine->GetProperty()->SetColor(1, 1, 0);
                    double vec[3];
                    vtkMath::Subtract(rotateAxisPtEnd, rotateAxisPtBegin, vec);
                    ren->AddActor(myLine);
                    vtkns::makeLines({ {0., 0., 0.}, {vec[0], vec[1], vec[2]} }, myLine);
                }
                // 新矩阵中的旋转轴
                {
                    std::array<double, 4> begin_, end_;
                    vtkns::Pt_t newpt0 = {
                        myMat->GetElement(0, 3),
                        myMat->GetElement(1, 3),
                        myMat->GetElement(2, 3)
                    };
                    vtkns::Pt_t newpt1;
                    {
                        double vec[3];
                        {
                            vtkMatrix4x4::MultiplyPoint(myMat->GetData(), rotateAxisPtBegin, begin_.data());
                            vtkMatrix4x4::MultiplyPoint(myMat->GetData(), rotateAxisPtEnd, end_.data());
                            vtkMath::Subtract(end_, begin_, vec);
                        }
                        vtkMath::Add(newpt0.data(), vec, newpt1.data());
                    }
                    static vtkNew<vtkActor> myLineOriginal; // 原始旋转轴映射到新矩阵中对应的线
                    static vtkNew<vtkActor> myLine; // 原始旋转轴映射到新矩阵中再移动到新矩阵中心对应的线
                    ren->AddActor(myLine);
                    ren->AddActor(myLineOriginal);
                    myLine->GetProperty()->SetOpacity(0.5);
                    vtkns::makeLines({newpt0, newpt1}, myLine);
                    vtkns::makeLines({{begin_[0],begin_[1],begin_[2]}, {end_[0],end_[1],end_[2]}}, myLineOriginal);
                }

                ImGui::TreePop();
            }

        vtkns::vtkObjSetup("M", myMat);

        auto f = [&](const double angle, const int axis = -1)
            {
                vtkNew<vtkTransform> trans;
                trans->SetMatrix(myMat);
                if (bPostMultiply)
                {
                    trans->PostMultiply();
                }
                else
                {
                    trans->PreMultiply();
                }

                if (-1 == axis && bRotateAroundCustomline)
                {
                    const double center[] =
                    {
                        rotateAxisPtBegin[0],
                        rotateAxisPtBegin[1],
                        rotateAxisPtBegin[2],
                    };
                    std::array<double, 3> arr;
                    vtkMath::Subtract(rotateAxisPtEnd, rotateAxisPtBegin, arr);
                    if (bPostMultiply)
                    {
                        trans->Translate(-center[0], -center[1], -center[2]);
                        trans->RotateWXYZ(angle, arr.data());
                        trans->Translate(center[0], center[1], center[2]);
                    }
                    else
                    {
                        trans->Translate(center[0], center[1], center[2]);
                        trans->RotateWXYZ(angle, arr.data());
                        trans->Translate(-center[0], -center[1], -center[2]);
                    }
                    myMat->DeepCopy(trans->GetMatrix());
                }
                else
                {
                    std::array<double, 3> arr;

                    switch (axis)
                    {
                    case -1:
                        vtkMath::Subtract(rotateAxisPtEnd, rotateAxisPtBegin, arr);
                        break;
                    case 0:
                        arr = { 1,0,0 };
                        break;
                    case 1:
                        arr = { 0,1,0 };
                        break;
                    case 2:
                        arr = { 0,0,1 };
                        break;
                    default:
                        break;
                    }

                    trans->RotateWXYZ(angle, arr.data());
                    myMat->DeepCopy(trans->GetMatrix());
                }
            };
        if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx("Rotate", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox(u8"在原始坐标系中运动(PostMultiply)", &bPostMultiply);
            vtkns::HelpMarkerSameLine(bPostMultiply ? u8R"(左乘，M_new=A*M，新的变换A是在世界坐标系中进行的)" : u8R"(右乘，M_new=M*A，新的变换A是在由M所定义的当前局部坐标系中进行的)");
            vtkns::ArrowButton("X", [&] { f(-1.8, 0); }, [&] { f(1.8, 0); });
            ImGui::SameLine();
            vtkns::ArrowButton("Y", [&] { f(-1.8, 1); }, [&] { f(1.8, 1); });
            ImGui::SameLine();
            vtkns::ArrowButton("Z", [&] { f(-1.8, 2); }, [&] { f(1.8, 2); });
            if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx("WXYZ", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox(u8"起止线", &bRotateAroundCustomline);
                vtkns::ArrowButton(bRotateAroundCustomline ? u8"围绕起止线旋转" : u8"将起止线移动至原点并绕其旋转", [&] { f(-1.8); }, [&] { f(1.8); });
            }
        }
    };

    AFTER_MY_CODE
}