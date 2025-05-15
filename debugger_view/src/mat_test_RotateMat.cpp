#include <ImGuiCommon.h>

int main()
{
    BEFORE_MY_CODE

    {
        vtkNew<vtkAxesActor> axes;
        ren->AddActor(axes);
        axes->SetAxisLabels(false);
        axes->GetXAxisShaftProperty()->SetOpacity(0.5);
        axes->GetYAxisShaftProperty()->SetOpacity(0.5);
        axes->GetZAxisShaftProperty()->SetOpacity(0.5);
        axes->SetNormalizedTipLength(0.0, 0.0, 0.0);
    }

    vtkNew<vtkLeaderActor2D> rotateAxis;
    {
        rotateAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
        rotateAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();
        rotateAxis->SetArrowStyleToOpen();
        rotateAxis->SetArrowPlacementToPoint2();
        rotateAxis->GetProperty()->SetColor(1, 1, 0);
        rotateAxis->GetProperty()->SetOpacity(0.3);
        ren->AddViewProp(rotateAxis);
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
            ImGui::Checkbox("PostMultiply ", &bPostMultiply);
            vtkns::HelpMarkerSameLine(bPostMultiply ? u8R"(左乘，M_new=A*M，新的变换A是在世界坐标系中进行的)" : u8R"(右乘，M_new=M*A，新的变换A是在由M所定义的当前局部坐标系中进行的)");

            static double rotateAxisPtBegin[4]{ 1.,0.,0., 1. };
            static double rotateAxisPtEnd[4]{ 1.,1.,1., 1. };
            if (ImGui::TreeNodeEx("RotateAxis", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragScalarN(u8"起", ImGuiDataType_Double, rotateAxisPtBegin, 3, 0.01f);
                ImGui::DragScalarN(u8"止", ImGuiDataType_Double, rotateAxisPtEnd, 3, 0.01f);
                rotateAxis->GetPositionCoordinate()->SetValue(rotateAxisPtBegin);
                rotateAxis->GetPosition2Coordinate()->SetValue(rotateAxisPtEnd);
                // 新矩阵中的旋转轴
                {
                    vtkns::Pt_t newpt0 = {
                        myMat->GetElement(0, 3),
                        myMat->GetElement(1, 3),
                        myMat->GetElement(2, 3)
                    };
                    vtkns::Pt_t newpt1;
                    {
                        double vec[3];
                        {
                            std::array<double, 4> pt0, pt1;
                            vtkMatrix4x4::MultiplyPoint(myMat->GetData(), rotateAxisPtBegin, pt0.data());
                            vtkMatrix4x4::MultiplyPoint(myMat->GetData(), rotateAxisPtEnd, pt1.data());
                            vtkMath::Subtract(pt1, pt0, vec);
                        }
                        vtkMath::Add(newpt0.data(), vec, newpt1.data());
                    }
                    static vtkNew<vtkActor> myLine;
                    ren->AddActor(myLine);
                    vtkns::makeLines({newpt0, newpt1}, myLine);
                }
                // 世界坐标中的旋转轴
                {
                    static vtkNew<vtkActor> myLine;
                    myLine->GetProperty()->SetOpacity(0.5);
                    double vec[3];
                    vtkMath::Subtract(rotateAxisPtEnd, rotateAxisPtBegin, vec);
                    ren->AddActor(myLine);
                    vtkns::makeLines({{0., 0., 0.}, {vec[0], vec[1], vec[2]}}, myLine);
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

                std::array<double, 3> arr;

                switch (axis)
                {
                case -1:
                    vtkMath::Subtract(rotateAxisPtEnd, rotateAxisPtBegin, arr);
                    break;
                case 0:
                    arr = {1,0,0};
                    break;
                case 1:
                    arr = {0,1,0};
                    break;
                case 2:
                    arr = {0,0,1};
                    break;
                default:
                    break;
                }

                trans->RotateWXYZ(angle, arr.data());
                myMat->DeepCopy(trans->GetMatrix());
            };
        if (ImGui::TreeNodeEx("Rotate", ImGuiTreeNodeFlags_DefaultOpen))
        {
            vtkns::ArrowButton("X", [&] { f(-1.8, 0); }, [&] { f(1.8, 0); });
            ImGui::SameLine();
            vtkns::ArrowButton("Y", [&] { f(-1.8, 1); }, [&] { f(1.8, 1); });
            ImGui::SameLine();
            vtkns::ArrowButton("Z", [&] { f(-1.8, 2); }, [&] { f(1.8, 2); });
            ImGui::SameLine();
            vtkns::ArrowButton("WXYZ", [&] { f(-1.8); }, [&] { f(1.8); });

            ImGui::TreePop();
        }
    };

    AFTER_MY_CODE
}