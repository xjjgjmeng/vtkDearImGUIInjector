﻿#include <ImGuiCommon.h>
#include <PolyDataHelper.h>

int main()
{
    BEFORE_MY_CODE
    vtkns::labelWorldZero(ren, false);

    vtkNew<vtkActor> actor;
    actor->GetProperty()->SetRenderPointsAsSpheres(1);
    actor->GetProperty()->SetPointSize(13);
    ren->AddViewProp(actor);

    vtkns::Pts_t pts;
    auto resetPts = [&]
        {
            pts.clear();
            for (auto i = 0; i < 3; ++i)
            {
                for (auto j = 0; j < 3; ++j)
                {
                    for (auto k = 0; k < 3; ++k)
                    {
                        pts.push_back({ double(i),double(j),double(k) });
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
        };
    resetPts();

    vtkNew<vtkActor> rotateAxis;
    ren->AddViewProp(rotateAxis);
    vtkns::Pts_t rotateAxisPts;
    rotateAxisPts.push_back({ double(0),double(0),double(0) });
    rotateAxisPts.push_back({ double(3),double(3),double(3) });
    vtkns::makeLines(rotateAxisPts, rotateAxis);

    vtkNew<vtkMatrix4x4> myMat;

    ::imgui_render_callback = [&]
        {
            if (ImGui::Button("ResetPts"))
            {
                vtkns::log(u8"点数据被重置");
                resetPts();
            }
            ImGui::SameLine();
            if (ImGui::Button("Identify"))
            {
                vtkns::log(u8"矩阵被重置为单位矩阵");
                myMat->Identity();
            }
            ImGui::SameLine();
            if (ImGui::Button("Apply"))
            {
                for (auto& i : pts)
                {
                    double oldPt[4]{ i[0], i[1], i[2], 1 };
                    double newPt[4];
                    myMat->MultiplyPoint(oldPt, newPt);
                    i[0] = newPt[0];
                    i[1] = newPt[1];
                    i[2] = newPt[2];
                }
                vtkns::makePoints(pts, actor);
            }
            ImGui::Text(vtkns::getMatrixString(myMat).c_str());
            if (double* v = rotateAxisPts.front().data(); ImGui::DragScalarN("RotateAxisPt1", ImGuiDataType_Double, v, 3, 0.01f))
            {
                vtkns::makeLines(rotateAxisPts, rotateAxis);
            }
            if (double* v = rotateAxisPts.back().data(); ImGui::DragScalarN("RotateAxisPt2", ImGuiDataType_Double, v, 3, 0.01f))
            {
                vtkns::makeLines(rotateAxisPts, rotateAxis);
            }
            // 平移
            {
                constexpr auto mystep = 0.1;
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
                        double arr[] = {
                            1.,0.,0., 0 == idx ? v : 0.,
                            0.,1.,0., 1 == idx ? v : 0.,
                            0.,0.,1., 2 == idx ? v : 0.,
                            0.,0.,0.,1.
                        };
                        double r[16];
                        vtkMatrix4x4::Multiply4x4(arr, myMat->GetData(), r);
                        myMat->DeepCopy(r);
                    };
                vtkns::ArrowButton(u8"平移X", [&] {f(0, -mystep); }, [&] {f(0, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"平移Y", [&] {f(1, -mystep); }, [&] {f(1, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"平移Z", [&] {f(2, -mystep); }, [&] {f(2, mystep); });
                ImGui::SameLine(); vtkns::HelpMarker(u8R"(坐标轴正方向为平移正方向)");
            }
            // 缩放
            {
                constexpr auto p = 2;
                constexpr auto n = 0.5;
                auto f = [&](const int idx, const double v)
                    {
                        switch (idx)
                        {
                        case 0:
                            vtkns::log(u8"沿着{}轴，缩放{}", "X", v);
                            break;
                        case 1:
                            vtkns::log(u8"沿着{}轴，缩放{}", "Y", v);
                            break;
                        case 2:
                            vtkns::log(u8"沿着{}轴，缩放{}", "Z", v);
                            break;
                        default:
                            break;
                        }
                        double arr[] = {
                            0==idx?v:1., 0., 0., 0.,
                            0., 1==idx?v:1., 0., 0.,
                            0., 0., 2==idx?v:1., 0.,
                            0., 0., 0., 1.
                        };
                        double r[16];
                        vtkMatrix4x4::Multiply4x4(arr, myMat->GetData(), r);
                        myMat->DeepCopy(r);
                    };
                vtkns::ArrowButton(u8"缩放X", [&] {f(0, n); }, [&] {f(0, p); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"缩放Y", [&] {f(1, n); }, [&] {f(1, p); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"缩放Z", [&] {f(2, n); }, [&] {f(2, p); });
            }
#if 0
            // Rodrigues' rotation formula
            {
                constexpr auto mystep = 0.1;
                auto f = [&](const double angle)
                    {
                        const auto& dstPt = rotateAxisPts.back();
                        const auto& srcPt = rotateAxisPts.front();
                        for (auto& i : pts)
                        {
                            double v[] = { i[0] - srcPt[0], i[1] - srcPt[1], i[2] - srcPt[2] };
                            decltype(v) r;
                            double q[] = { angle, dstPt[0] - srcPt[0], dstPt[1] - srcPt[1], dstPt[2] - srcPt[2] };
                            vtkMath::Normalize(&q[1]); // 须为单位向量
                            vtkMath::RotateVectorByWXYZ(v, q, r);
                            // vtkMath::RotateVectorByNormalizedQuaternion
                            i[0] = srcPt[0] + r[0];
                            i[1] = srcPt[1] + r[1];
                            i[2] = srcPt[2] + r[2];
                        }
                        vtkns::makePoints(pts, actor);
                    };
                {
                    ImGui::Text(u8"Rodrigues' rotation formula:");
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Rodrigues-", ImGuiDir_Left)) { f(-mystep); }
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    if (ImGui::ArrowButton("##Rodrigues+", ImGuiDir_Right)) { f(mystep); }
                    ImGui::PopButtonRepeat();
                }
                ImGui::SameLine();
                vtkns::HelpMarker(u8R"(参数v,q,r
q的第一个值的旋转角度，第二到四个值是旋转（单位）向量，方向是p1指向p2
v向量绕着q的二到四参数指定的单位旋转轴，旋转q的第一个参数指定的弧度，得到r向量
旋转正方向遵循右手定则
v和得到的r的长度一致)");
            }
#endif
            // 旋转
            {
                constexpr auto mystep = 3.14159/2/2/2/2;
                auto f = [&](const int idx, const double v)
                    {
                        switch (idx)
                        {
                        case 0:
                            vtkns::log(u8"沿着{}轴，旋转{}°", "X", vtkMath::DegreesFromRadians(v));
                            break;
                        case 1:
                            vtkns::log(u8"沿着{}轴，旋转{}°", "Y", vtkMath::DegreesFromRadians(v));
                            break;
                        case 2:
                            vtkns::log(u8"沿着{}轴，旋转{}°", "Z", vtkMath::DegreesFromRadians(v));
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
                        double r[16];
                        vtkMatrix4x4::Multiply4x4(arr.data(), myMat->GetData(), r);
                        myMat->DeepCopy(r);
                    };
                vtkns::ArrowButton(u8"旋转X", [&] {f(0, -mystep); }, [&] {f(0, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"旋转Y", [&] {f(1, -mystep); }, [&] {f(1, mystep); }); vtkns::ArrowButtonSameLine();
                vtkns::ArrowButton(u8"旋转Z", [&] {f(2, -mystep); }, [&] {f(2, mystep); });
                ImGui::SameLine(); vtkns::HelpMarker(u8R"(右手握住旋转轴，大拇指指向正方向，四指弯曲的方向为旋转的正方向
5.625°)");
            }
        };

    AFTER_MY_CODE
}