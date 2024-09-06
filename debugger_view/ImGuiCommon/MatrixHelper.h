#pragma once

namespace vtkns
{
	namespace mat
	{
        // inline?!
		inline void translate(vtkMatrix4x4* mat, const double x, const double y, const double z, const bool useVtkTransform)
		{
            if (useVtkTransform)
            {
                // 沿着[新坐标系]的轴移动
#if 0
                vtkNew<vtkTransform> transform;
                transform->SetMatrix(mat);
                transform->Translate(x, y, z);
                mat->DeepCopy(transform->GetMatrix());
#else
                double point[4];
                point[0] = x;
                point[1] = y;
                point[2] = z;
                point[3] = 1.0;
                double center[4];
                mat->MultiplyPoint(point, center);
                mat->SetElement(0, 3, center[0]);
                mat->SetElement(1, 3, center[1]);
                mat->SetElement(2, 3, center[2]);
#endif
            }
            else
            {
                // 沿着[旧坐标系]的轴移动
                double myMat[] = {
                    1.,0.,0.,x,
                    0.,1.,0.,y,
                    0.,0.,1.,z,
                    0.,0.,0.,1.
                };
                double r[16];
                vtkMatrix4x4::Multiply4x4(myMat, mat->GetData(), r);
                mat->DeepCopy(r);
            }
		}

        inline void scale(vtkMatrix4x4* mat, const double x, const double y, const double z, const bool useVtkTransform)
        {
            if (useVtkTransform)
            {
                // 沿着[新坐标系]的轴缩放
                vtkNew<vtkTransform> transform;
                transform->SetMatrix(mat);
                transform->Scale(x, y, z);
                mat->DeepCopy(transform->GetMatrix());
            }
            else
            {
                // 沿着[旧坐标系]的轴缩放
                double arr[] = {
                    x, 0., 0., 0.,
                    0., y, 0., 0.,
                    0., 0., z, 0.,
                    0., 0., 0., 1.
                };
                double r[16];
                vtkMatrix4x4::Multiply4x4(arr, mat->GetData(), r);
                mat->DeepCopy(r);
            }
        }

        inline void rotate(vtkMatrix4x4* mat, const int axis, const double degrees, const bool useVtkTransform)
        {
            if (useVtkTransform)
            {
                // 沿着[新坐标系]的轴旋转
                vtkNew<vtkTransform> transform;
                transform->SetMatrix(mat);
                switch (axis)
                {
                case 0:
                    transform->RotateX(degrees);
                    break;
                case 1:
                    transform->RotateY(degrees);
                    break;
                case 2:
                    transform->RotateZ(degrees);
                    break;
                default:
                    break;
                }
                mat->DeepCopy(transform->GetMatrix());
            }
            else
            {
                // 沿着[旧坐标系]的轴旋转
                std::array<double, 16> arr;
                const auto radian = vtkMath::RadiansFromDegrees(degrees);
                switch (axis)
                {
                case 0: // x
                    arr = std::array{
                        1., 0., 0., 0.,
                        0., std::cos(radian), -std::sin(radian), 0.,
                        0., std::sin(radian), std::cos(radian), 0.,
                        0., 0., 0., 1.
                    };
                    break;
                case 1: // y
                    arr = std::array{
                        std::cos(radian), 0., std::sin(radian), 0.,
                        0., 1., 0., 0.,
                        -std::sin(radian), 0., std::cos(radian), 0.,
                        0., 0., 0., 1.
                    };
                    break;
                case 2: // z
                    arr = std::array{
                        std::cos(radian), -std::sin(radian), 0., 0.,
                        std::sin(radian), std::cos(radian), 0., 0.,
                        0., 0., 1., 0.,
                        0., 0., 0., 1.
                    };
                    break;
                default:
                    break;
                }
                double r[16];
                vtkMatrix4x4::Multiply4x4(arr.data(), mat->GetData(), r);
                mat->DeepCopy(r);
            }
        }

        inline void genAxes(vtkRenderer* ren, vtkMatrix4x4* mat)
        {
            struct Actors
            {
                vtkNew<vtkActor> originActor;
                vtkNew<vtkActor> x;
                vtkNew<vtkActor> y;
                vtkNew<vtkActor> z;
                vtkNew<vtkActor> xoyPlaneActor;
            };
            static std::map<vtkMatrix4x4*, Actors> mymap;
            if (0 == mymap.count(mat))
            {
                auto& originActor = mymap[mat].originActor;
                originActor->GetProperty()->SetColor(1, 1, 0);
                originActor->GetProperty()->SetPointSize(11);
                originActor->GetProperty()->SetRenderPointsAsSpheres(1);
                ren->AddViewProp(originActor);

                struct
                {
                    vtkActor* acotr;
                    double color[3];
                } x{ mymap[mat].x, {1,0,0} }, y{ mymap[mat].y, {0,1,0} }, z{ mymap[mat].z, {0,0,1} };
                for (auto& [actor, color] : { x, y, z })
                {
                    actor->GetProperty()->SetColor(const_cast<double*>(color));
                    actor->GetProperty()->SetLineWidth(3);
                    actor->GetProperty()->SetRenderLinesAsTubes(1);
                    ren->AddViewProp(actor);
                }

                auto& xoyPlaneActor = mymap[mat].xoyPlaneActor;
                //::xoyPlaneActor->GetProperty()->SetOpacity(.8);
#if 1
                xoyPlaneActor->GetProperty()->SetRepresentationToWireframe();
#else
                xoyPlaneActor->GetProperty()->SetRepresentationToSurface();
                xoyPlaneActor->GetProperty()->SetOpacity(.3);
#endif
                xoyPlaneActor->GetProperty()->SetColor(0, 1, 0);
                ren->AddViewProp(xoyPlaneActor);
            }

            {
                // origin
                vtkns::makePoints(vtkns::Pts_t{ {mat->GetElement(0, 3), mat->GetElement(1, 3), mat->GetElement(2, 3)} }, mymap[mat].originActor);
                // xyz
                constexpr auto len = 180;
                double oPt[4] = { 0,0,0,1 };
                double xPt[4] = { len,0,0,1 };
                double yPt[4] = { 0,len,0,1 };
                double zPt[4] = { 0,0,len,1 };
                for (auto& i : { oPt, xPt, yPt, zPt })
                {
                    mat->MultiplyPoint(i, i);
                }
                vtkns::makeLines(vtkns::Pts_t{ {oPt[0], oPt[1], oPt[2]}, {xPt[0], xPt[1], xPt[2]} }, mymap[mat].x);
                vtkns::makeLines(vtkns::Pts_t{ {oPt[0], oPt[1], oPt[2]}, {yPt[0], yPt[1], yPt[2]} }, mymap[mat].y);
                vtkns::makeLines(vtkns::Pts_t{ {oPt[0], oPt[1], oPt[2]}, {zPt[0], zPt[1], zPt[2]} }, mymap[mat].z);
                // xoy
                vtkNew<vtkPlaneSource> src;
                src->SetPoint1(xPt);
                src->SetPoint2(yPt);
                src->SetOrigin(oPt);
                src->SetCenter(oPt);
                src->SetXResolution(23);
                src->SetYResolution(23);
                vtkNew<vtkPolyDataMapper> mapper;
                mapper->SetInputConnection(src->GetOutputPort());
                mymap[mat].xoyPlaneActor->SetMapper(mapper);
            }
        }
	}
}