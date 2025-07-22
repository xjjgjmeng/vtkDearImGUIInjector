#include <ImGuiCommon.h>

// 矩阵作用在点上的运动
int main()
{
    BEFORE_MY_CODE
    vtkns::labelWorldZero(ren, false);

    vtkNew<vtkActor> actor;
    actor->GetProperty()->SetRenderPointsAsSpheres(1);
    actor->GetProperty()->SetPointSize(13);
    ren->AddViewProp(actor);

    vtkns::Pts_t pts;
    for (auto i = 0; i < 3; ++i)
    {
        for (auto j = 0; j < 3; ++j)
        {
            for (auto k = 0; k < 3; ++k)
            {
                pts.push_back({ double(i),double(j),double(k)});
            }
        }
    }
    pts.push_back({ double(3),double(0),double(0) });
    pts.push_back({ double(0),double(3),double(0) });
    pts.push_back({ double(0),double(4),double(0) });
    pts.push_back({ double(0),double(0),double(3) });
    pts.push_back({ double(0),double(0),double(4) });
    pts.push_back({ double(0),double(0),double(5) });
    vtkns::makePoints(pts, actor);

    vtkNew<vtkLeaderActor2D> rotateAxis;
    {
        rotateAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
        rotateAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();
        rotateAxis->SetArrowStyleToOpen();
        rotateAxis->SetArrowPlacementToPoint2();
        rotateAxis->GetProperty()->SetColor(1,1,0);
    }
    ren->AddViewProp(rotateAxis);

    ::imgui_render_callback = [&]
        {
            if (ImGui::TreeNodeEx(u8"左右乘对比"))
            {
                static std::pair<vtkNew<vtkMatrix4x4>, vtkNew<vtkMatrix4x4>> leftrightmutiply;
                vtkns::vtkObjSetup("A", leftrightmutiply.first);
                vtkns::vtkObjSetup("B", leftrightmutiply.second);

                vtkNew<vtkMatrix4x4> retMat;
                vtkMatrix4x4::Multiply4x4(leftrightmutiply.first, leftrightmutiply.second, retMat);
                vtkns::ImGuiText("A*B:\n{}", vtkns::getMatrixString(retMat));
                vtkMatrix4x4::Multiply4x4(leftrightmutiply.second, leftrightmutiply.first, retMat);
                vtkns::ImGuiText("B*A:\n{}", vtkns::getMatrixString(retMat));

                ImGui::TreePop();
            }

            // 缩放
            {
                constexpr auto p = 1.1;
                constexpr auto n = 0.9;
                auto f = [&](const int idx, const double v)
                {
                    const double mat[] = {
                        0==idx?v:1., 0., 0., 0.,
                        0., 1==idx?v:1., 0., 0.,
                        0., 0., 2==idx?v:1., 0.,
                        0., 0., 0., 1.
                    };
                    for (auto& i : pts)
                    {
                        const double inPt[4]{ i[0], i[1], i[2], 1 };
                        double outPt[4];
                        vtkMatrix4x4::MultiplyPoint(mat, inPt, outPt);
                        i[0] = outPt[0];
                        i[1] = outPt[1];
                        i[2] = outPt[2];
                    }
                    vtkns::makePoints(pts, actor);
                };
                vtkns::ArrowButton(u8"缩放X", [&]{ f(0, n); }, [&] { f(0, p); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"缩放Y", [&]{ f(1, n); }, [&] { f(1, p); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"缩放Z", [&]{ f(2, n); }, [&] { f(2, p); });
            }
            // 平移
            {
                constexpr auto mystep = 0.1;
                auto f = [&](const int idx, const double v)
                    {
                        const double mat[] = {
                            1.,0.,0., 0==idx?v:0.,
                            0.,1.,0., 1==idx?v:0.,
                            0.,0.,1., 2==idx?v:0.,
                            0.,0.,0.,1.
                        };
                        for (auto& i : pts)
                        {
                            const double inPt[4]{i[0], i[1], i[2], 1};
                            double outPt[4];
                            vtkMatrix4x4::MultiplyPoint(mat, inPt, outPt);
                            i[0] = outPt[0];
                            i[1] = outPt[1];
                            i[2] = outPt[2];
                        }
                        vtkns::makePoints(pts, actor);
                    };
                vtkns::ArrowButton(u8"平移X", [&] { f(0, -mystep); }, [&] { f(0, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"平移Y", [&] { f(1, -mystep); }, [&] { f(1, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"平移Z", [&] { f(2, -mystep); }, [&] { f(2, mystep); });
                ImGui::SameLine(); vtkns::HelpMarker(u8R"(坐标轴正方向为平移正方向)");
            }
            // 旋转
            {
                constexpr auto mystep = 3.14159/2/2/2/2;
                auto f = [&](const int idx, const double v)
                    {
                        switch (idx)
                        {
                        case 0:
                            vtkns::log(u8"沿着{}轴，平移{}", "X", v);
                            break;
                        case 1:
                            vtkns::log(u8"沿着{}轴，平移{}", "Y", v);
                            break;
                        case 2:
                            vtkns::log(u8"沿着{}轴，平移{}", "Z", v);
                            break;
                        default:
                            break;
                        }
                        std::array<double, 16> arr;
                        switch (idx)
                        {
                        case 0: // x
                            arr = std::array{
                                1., 0., 0., 0.,
                                0., std::cos(v), -std::sin(v), 0.,
                                0., std::sin(v), std::cos(v), 0.,
                                0., 0., 0., 1.
                            };
                            break;
                        case 1: // y
                            arr = std::array{
                                std::cos(v), 0., std::sin(v), 0.,
                                0., 1., 0., 0.,
                                -std::sin(v), 0., std::cos(v), 0.,
                                0., 0., 0., 1.
                            };
                            break;
                        case 2: // z
                            arr = std::array{
                                std::cos(v), -std::sin(v), 0., 0.,
                                std::sin(v), std::cos(v), 0., 0.,
                                0., 0., 1., 0.,
                                0., 0., 0., 1.
                            };
                            break;
                        default:
                            break;
                        }
                        for (auto& i : pts)
                        {
                            const double inPt[4]{ i[0], i[1], i[2], 1 };
                            double outPt[4];
                            vtkMatrix4x4::MultiplyPoint(arr.data(), inPt, outPt);
                            i[0] = outPt[0];
                            i[1] = outPt[1];
                            i[2] = outPt[2];
                        }
                        vtkns::makePoints(pts, actor);
                    };
                vtkns::ArrowButton(u8"旋转X", [&] { f(0, -mystep); }, [&] { f(0, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"旋转Y", [&] { f(1, -mystep); }, [&] { f(1, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"旋转Z", [&] { f(2, -mystep); }, [&] { f(2, mystep); });
                ImGui::SameLine(); vtkns::HelpMarker(u8R"(右手握住旋转轴，大拇指指向正方向，四指弯曲的方向为旋转的正方向)");
            }

            if (ImGui::TreeNodeEx("Rodrigues' rotation formula", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static vtkns::Pts_t rotateAxisPts{ {0,0,0}, {3,3,3} };
                if (ImGui::TreeNodeEx("RotateAxis", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::DragScalarN("p1", ImGuiDataType_Double, rotateAxisPts.front().data(), 3, 0.01f);
                    ImGui::DragScalarN("p2", ImGuiDataType_Double, rotateAxisPts.back().data(), 3, 0.01f);
                    rotateAxis->GetPositionCoordinate()->SetValue(rotateAxisPts.front().data());
                    rotateAxis->GetPosition2Coordinate()->SetValue(rotateAxisPts.back().data());
                    ImGui::SameLine(); vtkns::HelpMarker(u8"p1是旋转中心(或p2)，旋转轴的方向由p1指向p2，且需为单位向量");

                    ImGui::TreePop();
                }

                constexpr auto mystep = 0.1;
                auto f = [&](const double angle)
                {
                    const auto& p2 = rotateAxisPts.back();
                    const auto& p1 = rotateAxisPts.front();
                    for (auto& i : pts)
                    {
                        double v[] = { i[0] - p1[0], i[1] - p1[1], i[2] - p1[2] };
                        decltype(v) r;
                        double q[] = { angle, p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
                        vtkMath::Normalize(&q[1]); // 须为单位向量
                        vtkMath::RotateVectorByWXYZ(v, q, r);
                        // vtkMath::RotateVectorByNormalizedQuaternion
                        i[0] = p1[0] + r[0];
                        i[1] = p1[1] + r[1];
                        i[2] = p1[2] + r[2];
                    }
                    vtkns::makePoints(pts, actor);
                };
                vtkns::ArrowButton(u8"逆向(顺时针)/正向(逆时针)", [&] { f(-mystep); }, [&] { f(mystep); });
                ImGui::SameLine(); vtkns::HelpMarker(u8R"(参数v,q,r
q的第一个值的旋转角度（弧度），第二到四个值是旋转向量（需要是单位向量），方向是p1指向p2
v向量绕着q的二到四参数指定的单位旋转轴，旋转q的第一个参数指定的弧度，得到r向量
旋转正方向遵循右手定则（右手握住旋转轴，大拇指指向向量所指的方向，其他手指所指方向为旋转正方向）
v和得到的r的长度一致)");

                ImGui::TreePop();
            }
        };

    AFTER_MY_CODE
}