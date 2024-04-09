﻿#include "ImGuiCommon.h"

namespace vtkns
{
    void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}

auto getMatrixString(vtkMatrix4x4* obj)
{
#if 1
    return fmt::format("Element:\n\t{:.5f}\t{:.5f}\t{:.5f}\t{:.5f}\n\t{:.5f}\t{:.5f}\t{:.5f}\t{:.5f}\n\t{:.5f}\t{:.5f}\t{:.5f}\t{:.5f}\n\t{:.5f}\t{:.5f}\t{:.5f}\t{:.5f}",
        obj->GetElement(0, 0), obj->GetElement(0, 1), obj->GetElement(0, 2), obj->GetElement(0, 3),
        obj->GetElement(1, 0), obj->GetElement(1, 1), obj->GetElement(1, 2), obj->GetElement(1, 3),
        obj->GetElement(2, 0), obj->GetElement(2, 1), obj->GetElement(2, 2), obj->GetElement(2, 3),
        obj->GetElement(3, 0), obj->GetElement(3, 1), obj->GetElement(3, 2), obj->GetElement(3, 3));
#else
    return fmt::format("Element:\n\t{}",
        *(double(*)[4][4])(obj->GetData()));
#endif
}

namespace
{
    void vtkElevationFilter_setup(vtkElevationFilter* obj)
    {
        if (double v[3]; obj->GetLowPoint(v), ImGui::DragScalarN("LowPoint", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01f))
        {
            obj->SetLowPoint(v);
        }
        if (double v[3]; obj->GetHighPoint(v), ImGui::DragScalarN("HighPoint", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01f))
        {
            obj->SetHighPoint(v);
        }
    }

    void vtkDataSetAlgorithm_setup(vtkDataSetAlgorithm* obj)
    {
    
    }

    void vtkImageToPolyDataFilter_setup(vtkImageToPolyDataFilter* obj)
    {
        if (ImGui::TreeNodeEx("OutputStyle", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Pixelize"))
            {
                obj->SetOutputStyleToPixelize();
            }
            ImGui::SameLine();
            if (ImGui::Button("Polygonalize"))
            {
                obj->SetOutputStyleToPolygonalize();
            }
            ImGui::SameLine();
            if (ImGui::Button("RunLength"))
            {
                obj->SetOutputStyleToRunLength();
            }
            ImGui::SameLine();
            ImGui::Text(fmt::format("{}", obj->GetOutputStyle()).c_str());

            ImGui::TreePop();
        }

        if (bool v = obj->GetSmoothing(); ImGui::Checkbox("Smoothing", &v))
        {
            obj->SetSmoothing(v);
        }

        if (bool v = obj->GetDecimation(); ImGui::Checkbox("Decimation", &v))
        {
            obj->SetDecimation(v);
        }

        if (int v = obj->GetSubImageSize(); ImGui::DragInt("SubImageSize", &v))
        {
            obj->SetSubImageSize(v);
        }

        if (int v = obj->GetNumberOfSmoothingIterations(); ImGui::DragInt("NumberOfSmoothingIterations", &v))
        {
            obj->SetNumberOfSmoothingIterations(v);
        }
    }

    void vtkPlaneSource_setup(vtkPlaneSource* obj)
    {
        if (int v = obj->GetOutputPointsPrecision(); ImGui::DragInt("OutputPointsPrecision", &v))
        {
            obj->SetOutputPointsPrecision(v);
        }
        if (int v = obj->GetXResolution(); ImGui::DragInt("XResolution", &v))
        {
            obj->SetXResolution(v);
        }
        if (int v = obj->GetYResolution(); ImGui::DragInt("YResolution", &v))
        {
            obj->SetYResolution(v);
        }
        {
            ImGui::Text("Push:");
            ImGui::SameLine();
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##-", ImGuiDir_Left)) { obj->Push(-1); }
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::ArrowButton("##+", ImGuiDir_Right)) { obj->Push(1); }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            vtkns::HelpMarker(u8R"(沿着法向前进或后退)");
        }
        if (double v[3]; obj->GetPoint1(v), ImGui::DragScalarN("Point1", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetPoint1(v);
        }
        if (double v[3]; obj->GetPoint2(v), ImGui::DragScalarN("Point2", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetPoint2(v);
        }
        if (double v[3]; obj->GetOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetOrigin(v);
        }
        if (double v[3]; obj->GetCenter(v), ImGui::DragScalarN("Center", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetCenter(v);
        }
        ImGui::SameLine();
        vtkns::HelpMarker(u8R"(移动整个plane，plane的size不变，Point1和Point2和Origin会被修改)");
        if (double v[3]; obj->GetNormal(v), ImGui::DragScalarN("Normal", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01f))
        {
            obj->SetNormal(v);
        }
    }

    void vtkFlyingEdges3D_setup(vtkFlyingEdges3D* obj)
    {
        if (double v = obj->GetValue(0); ImGui::InputDouble("Value", &v, 100.f, 100.0f, "%.8f"))
        {
            obj->SetValue(0, v);
        }
    }

    void vtkMatrix4x4_setup(vtkMatrix4x4* obj)
    {
    }

    void vtkAbstractImageInterpolator_setup(vtkAbstractImageInterpolator* obj)
    {
        if (bool v = obj->GetSlidingWindow(); ImGui::Checkbox("SlidingWindow", &v))
        {
            obj->SetSlidingWindow(v);
        }
        ImGui::Text(fmt::format("OutValue: {}", obj->GetOutValue()).c_str());
        {
            double v[3];
            obj->GetSpacing(v);
            ImGui::Text(fmt::format("Spacing: {::.2f}", v).c_str());
        }
        {
            double v[3];
            obj->GetOrigin(v);
            ImGui::Text(fmt::format("Origin: {::.2f}", v).c_str());
        }
        {
            int v[6];
            obj->GetExtent(v);
            ImGui::Text(fmt::format("Extent: {}", v).c_str());
        }

        {
            ImGui::Text(fmt::format("BorderMode: {}", obj->GetBorderModeAsString()).c_str()); ImGui::SameLine();
            if (ImGui::Button("Clamp")) obj->SetBorderModeToClamp(); ImGui::SameLine();
            if (ImGui::Button("Mirror")) obj->SetBorderModeToMirror(); ImGui::SameLine();
            if (ImGui::Button("Repeat")) obj->SetBorderModeToRepeat();
        }

        {
            static double v[3];
            static bool b = false;
            ImGui::Text(fmt::format("{}", b).c_str());
            ImGui::SameLine();
            if (ImGui::DragScalarN("CheckBoundsIJK", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 1.f))
            {
                b = obj->CheckBoundsIJK(v);
            }
        }
        {
            static double v[3];
            static bool b = false;
            static double r[1];
            ImGui::Text(fmt::format("{} {::.2f}", b, r).c_str());
            ImGui::SameLine();
            if (ImGui::DragScalarN("Interpolate", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 1.f))
            {
                b = obj->Interpolate(v, r);
            }
        }
    }

    void vtkImageReslice_setup(vtkImageReslice* obj)
    {
        if (ImGui::TreeNodeEx("Slab", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // slab mode
            {
                const char* slabModeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
                auto currentSlabMode = obj->GetSlabMode();
                if (ImGui::Combo("Mode", &currentSlabMode, slabModeText, IM_ARRAYSIZE(slabModeText)))
                {
                    obj->SetSlabMode(currentSlabMode);
                }
            }

            // thickness
            {
                auto numOfSlices = obj->GetSlabNumberOfSlices();
                if (ImGui::DragInt("NumberOfSlices", &numOfSlices, 1, 1, 10000))
                {
                    obj->SetSlabNumberOfSlices(numOfSlices);
                }
            }

            if (double v = obj->GetSlabSliceSpacingFraction(); ImGui::DragScalar("SliceSpacingFraction", ImGuiDataType_Double, &v))
            {
                obj->SetSlabSliceSpacingFraction(v);
            }

            if (bool v = obj->GetSlabTrapezoidIntegration(); ImGui::Checkbox("TrapezoidIntegration", &v))
            {
                obj->SetSlabTrapezoidIntegration(v);
            }

            ImGui::TreePop();
        }

        {
            if (auto b = ImGui::TreeNodeEx("ResliceAxes", ImGuiTreeNodeFlags_DefaultOpen); ImGui::SameLine(), vtkns::HelpMarker(u8R"(1.将reslice坐标系放到origin指定的位置
2.根据xoy平面reslice一个slice
3.根据设置的output相关的origin，spacing和extent选取此slice的一部分
4.将最终得到vtkImageData在旧世界中呈现)"), b)
            {
                double xyz[9];
                obj->GetResliceAxesDirectionCosines(xyz);
                if (ImGui::DragScalarN("DirectionCosinesX", ImGuiDataType_Double, xyz, 3, .01f))
                {
                    obj->SetResliceAxesDirectionCosines(xyz);
                }
                if (ImGui::DragScalarN("DirectionCosinesY", ImGuiDataType_Double, xyz + 3, 3, .01f))
                {
                    obj->SetResliceAxesDirectionCosines(xyz);
                }
                if (ImGui::DragScalarN("DirectionCosinesZ", ImGuiDataType_Double, xyz + 6, 3, .01f))
                {
                    obj->SetResliceAxesDirectionCosines(xyz);
                }

                if (double v[3]; obj->GetResliceAxesOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .1f))
                {
                    obj->SetResliceAxesOrigin(v);
                } ImGui::SameLine(); vtkns::HelpMarker(u8R"(指定reslice坐标系的origin在旧世界中的位置
调节该值相当于reslice坐标系在旧世界中移动
xoy应该可以穿过imagedata，不然无输出
-固定output的origin和extent不变，修改ResliceAxes的origin的x和y，发现黑窗在世界中不动，image在黑窗口中游走
-固定ResliceAxes的origin，修改output的origin，发现黑窗在世界中游走，可以看到image不同部分
-固定ResliceAxes的origin，修改output的extent，发现黑窗在世界扩大或缩小，可以看到image的更多或更少
output的origin是相对于新坐标系的，把新坐标系的origin处看作（0，0，0），但最后在原始的坐标系中也是一样的)");

                {
                    auto f = [](vtkMatrix4x4* mat, const double v)
                    {
                        vtkNew<vtkTransform> transform;
                        transform->SetMatrix(mat);
                        transform->RotateZ(v);
                        mat->DeepCopy(transform->GetMatrix());
                    };
                    ImGui::Text("RotateZ:");
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Z-", ImGuiDir_Left)) { f(obj->GetResliceAxes(), -5); }
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    if (ImGui::ArrowButton("##Z+", ImGuiDir_Right)) { f(obj->GetResliceAxes(), 5); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(每次绕着Z轴向左或向右旋转5°)");
                }
                {
                    auto f = [](vtkMatrix4x4* mat, const double v)
                    {
                        vtkNew<vtkTransform> transform;
                        transform->SetMatrix(mat);
                        transform->RotateY(v);
                        mat->DeepCopy(transform->GetMatrix());
                    };
                    ImGui::Text("RotateY:");
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Y-", ImGuiDir_Left)) { f(obj->GetResliceAxes(), -5); }
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    if (ImGui::ArrowButton("##Y+", ImGuiDir_Right)) { f(obj->GetResliceAxes(), 5); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(每次绕着Y轴向左或向右旋转5°)");
                }
                {
                    auto f = [](vtkMatrix4x4* mat, const double v)
                    {
                        vtkNew<vtkTransform> transform;
                        transform->SetMatrix(mat);
                        transform->RotateX(v);
                        mat->DeepCopy(transform->GetMatrix());
                    };
                    ImGui::Text("RotateX:");
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##X-", ImGuiDir_Left)) { f(obj->GetResliceAxes(), -5); }
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    if (ImGui::ArrowButton("##X+", ImGuiDir_Right)) { f(obj->GetResliceAxes(), 5); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(每次绕着X轴向左或向右旋转5°)");
                }

                ImGui::Text(::getMatrixString(obj->GetResliceAxes()).c_str());

                ImGui::TreePop();
            }
        }

        // output
        if (ImGui::TreeNodeEx("Output", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (double v[3]; obj->GetOutputOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
            {
                obj->SetOutputOrigin(v);
                obj->Update(); // 没有此句会输出的都是二维
            } ImGui::SameLine(); vtkns::HelpMarker(u8R"(将reslice坐标系的origin处当中新世界的（0，0，0）
如果输出的是2维，调节z无效，因为只reslice出一张图，所以z被忽略了？？输出图像只能在一个平面上游移)");
            //::reslice->GetOutputInformation(0)->Get(vtkDataObject::SPACING(), myArray); // 0.25
            if (double v[3]; obj->GetOutputSpacing(v), ImGui::DragScalarN("Spacing", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
            {
                obj->SetOutputSpacing(v);
                obj->Update(); // 没有此句会输出的都是二维
            } ImGui::SameLine(); vtkns::HelpMarker(u8R"(和vtkImageData的spacing不同。
通过调大spacing再配合网格可以看到虽然输出的图像尺寸变大了，但是对应像素的位置是不变的，通过采样？？
确定了spacing就相当于确定了网格，后面的extent只是选取想要的网格索引来进一步reslice)");
            if (int v[6]; obj->GetOutputExtent(v), ImGui::DragScalarN("Extent", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
            {
                obj->SetOutputExtent(v);
                obj->Update(); // 没有此句会输出的都是二维
            } ImGui::SameLine(); vtkns::HelpMarker(u8R"(新的slice已经切割出来，此属性控制可以看到的图像范围)");

            if (ImGui::Button("SetOutputOriginToDefault"))
            {
                obj->SetOutputOriginToDefault();
            }
            ImGui::SameLine();
            if (ImGui::Button("SetOutputExtentToDefault"))
            {
                obj->SetOutputExtentToDefault();
            }

            {
                ImGui::Text("Dimensionality");
                ImGui::SameLine();
                int v = obj->GetOutputDimensionality();
                ImGui::RadioButton("2##OutputDimensionality", &v, 2); ImGui::SameLine();
                ImGui::RadioButton("3##OutputDimensionality", &v, 3);
                if (obj->GetOutputDimensionality() != v)
                {
                    obj->SetOutputDimensionality(v);
                    obj->Update(); // 没有此句会输出的都是二维
                }
            }
            // InterpolationMode
            {
                ImGui::Text("InterpolationMode");
                ImGui::SameLine();
                int v = obj->GetInterpolationMode();
                ImGui::RadioButton("NearestNeighbor", &v, VTK_RESLICE_NEAREST); ImGui::SameLine();
                ImGui::RadioButton("Linear", &v, VTK_RESLICE_LINEAR); ImGui::SameLine();
                ImGui::RadioButton("Cubic", &v, VTK_RESLICE_CUBIC);
                if (obj->GetInterpolationMode() != v)
                {
                    obj->SetInterpolationMode(v);
                }
            }

            if (ImGui::TreeNodeEx("Scalar", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (double v = obj->GetScalarShift(); ImGui::DragScalar("Shift", ImGuiDataType_Double, &v))
                {
                    obj->SetScalarShift(v);
                }

                if (double v = obj->GetScalarScale(); ImGui::DragScalar("Scale", ImGuiDataType_Double, &v))
                {
                    obj->SetScalarScale(v);
                }

                for (auto i : { VTK_CHAR, VTK_SIGNED_CHAR, VTK_UNSIGNED_CHAR,VTK_SHORT, VTK_UNSIGNED_SHORT, VTK_INT, VTK_UNSIGNED_INT, VTK_FLOAT, VTK_DOUBLE })
                {
                    if (ImGui::Button(fmt::format("{}", vtkImageScalarTypeNameMacro(i)).c_str()))
                    {
                        obj->SetOutputScalarType(i);
                    }
                    ImGui::SameLine();
                }
                ImGui::Text(fmt::format("Type: {}", vtkImageScalarTypeNameMacro(obj->GetOutputScalarType())).c_str());

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        {
            if (auto wrap = obj->GetWrap(); ImGui::Button(fmt::format("Wrap: {}", wrap).c_str()))
            {
                obj->SetWrap(!wrap);
            }
            ImGui::SameLine();
            if (auto mirror = obj->GetMirror(); ImGui::Button(fmt::format("Mirror: {}", mirror).c_str()))
            {
                obj->SetMirror(!mirror);
            }
            ImGui::SameLine();
            if (bool v = obj->GetGenerateStencilOutput(); ImGui::Checkbox("GenerateStencilOutput", &v))
            {
                obj->SetGenerateStencilOutput(v);
            }
            ImGui::SameLine();
            if (bool v = obj->GetInterpolate(); ImGui::Checkbox("Interpolate", &v))
            {
                obj->SetInterpolate(v);
            }
        }

        if (ImGui::TreeNodeEx("Border", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (bool v = obj->GetBorder(); ImGui::Checkbox("On", &v))
            {
                obj->SetBorder(v);
            }
            if (double v = obj->GetBorderThickness(); ImGui::DragScalar("Thickness", ImGuiDataType_Double, &v))
            {
                obj->SetBorderThickness(v);
            }

            ImGui::TreePop();
        }

        if (bool v = obj->GetAutoCropOutput(); ImGui::Checkbox("AutoCropOutput", &v))
        {
            obj->SetAutoCropOutput(v);
        }

        if (bool v = obj->GetOptimization(); ImGui::Checkbox("Optimization", &v))
        {
            obj->SetOptimization(v);
        }

        if (ImGui::TreeNodeEx("Background"))
        {
            if (float v[4] = { obj->GetBackgroundColor()[0],obj->GetBackgroundColor()[1],obj->GetBackgroundColor()[2],obj->GetBackgroundColor()[3] }; ImGui::ColorEdit4("Color", v))
            {
                obj->SetBackgroundColor(v[0], v[1], v[2], v[3]);
            }

            if (double v = obj->GetBackgroundLevel(); ImGui::DragScalar("Level", ImGuiDataType_Double, &v))
            {
                obj->SetBackgroundLevel(v);
            }

            ImGui::TreePop();
        }

        vtkns::vtkObjSetup("Interpolator", obj->GetInterpolator());
    }

    void vtkMarchingCubes_setup(vtkMarchingCubes* obj)
    {
        if (double v = obj->GetValue(0); ImGui::InputDouble("Value", &v, 100.f, 100.0f, "%.8f"))
        {
            obj->SetValue(0, v);
        }
    }

    void vtkTransformPolyDataFilter_setup(vtkTransformPolyDataFilter* obj)
    {
    }

    void vtkDataObject_setup(vtkDataObject* obj)
    {
        ImGui::Text(fmt::format("ActualMemorySize: {}", obj->GetActualMemorySize()).c_str());
        ImGui::Text(fmt::format("DataObjectType: {}", obj->GetDataObjectType()).c_str());
        ImGui::Text(fmt::format("ExtentType: {}", obj->GetExtentType()).c_str());
    }

    void vtkDataSet_setup(vtkDataSet* obj)
    {
        ImGui::Text(fmt::format("NumberOfPoints: {}", obj->GetNumberOfPoints()).c_str());
        ImGui::Text(fmt::format("NumberOfCells: {}", obj->GetNumberOfCells()).c_str());
        ImGui::Text(fmt::format("Length: {}", obj->GetLength()).c_str());
        ImGui::Text(fmt::format("Length2: {}", obj->GetLength2()).c_str());
        ImGui::Text(fmt::format("HasAnyBlankPoints: {}", obj->HasAnyBlankPoints()).c_str());
        ImGui::Text(fmt::format("HasAnyBlankCells: {}", obj->HasAnyBlankCells()).c_str());
        ImGui::Text(fmt::format("HasAnyGhostPoints: {}", obj->HasAnyGhostPoints()).c_str());
        ImGui::Text(fmt::format("HasAnyGhostCells: {}", obj->HasAnyGhostCells()).c_str());
        if (ImGui::Button("ComputeBounds")) obj->ComputeBounds();
        //ImGui::Text(fmt::format("NumberOfElements: {}", obj->GetNumberOfElements()).c_str());
        {
            double v[6];
            obj->GetBounds(v);
            ImGui::Text(fmt::format("Bounds: {::.2f}", v).c_str());
        }
        {
            double v[3];
            obj->GetCenter(v);
            ImGui::Text(fmt::format("Center: {::.2f}", v).c_str());
        }
        {
            double v[2];
            obj->GetScalarRange(v);
            ImGui::Text(fmt::format("ScalarRange: {::.2f}", v).c_str());
        }
    }

    void vtkPointSet_setup(vtkPointSet* obj)
    {
        if (ImGui::TreeNodeEx("Points"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
        {
            for (auto i = 0; i < obj->GetNumberOfPoints(); ++i)
            {
                ImGui::Text(fmt::format("point: {} {} {}", obj->GetPoint(i)[0], obj->GetPoint(i)[1], obj->GetPoint(i)[2]).c_str());
            }
            ImGui::TreePop();
        }
    }

    void vtkImageData_setup(vtkImageData* obj)
    {
        ImGui::Text(fmt::format("DataDimension: {}", obj->GetDataDimension()).c_str());
        ImGui::Text(fmt::format("ScalarType: {}", obj->GetScalarTypeAsString()).c_str());
        ImGui::Text(fmt::format("ScalarSize: {}", obj->GetScalarSize()).c_str());
        {
            int v[3];
            obj->GetDimensions(v);
            ImGui::Text(fmt::format("Dimensions: {}", v).c_str());
        }
        {
            int v[3];
            obj->GetCellDims(v);
            ImGui::Text(fmt::format("CellDims: {}", v).c_str());
        }
        if (double v[3]; obj->GetOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetOrigin(v);
        }
        if (double v[3]; obj->GetSpacing(v), ImGui::DragScalarN("Spacing", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetSpacing(v);
        } ImGui::SameLine(); vtkns::HelpMarker(u8"调节spacing可以看到图像伸缩变形。对比vtkImageRelice的spacing");
        if (int v[6]; obj->GetExtent(v), ImGui::DragScalarN("Extent", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
        {
            obj->SetExtent(v);
        }
        {
            static int v[3]{};
            static int r{};
            ImGui::DragScalarN(std::to_string(r).c_str(), ImGuiDataType_S32, v, IM_ARRAYSIZE(v)); // 这里id有问题！！
            ImGui::SameLine();
            if (ImGui::Button("GetScalarIndex")) r = obj->GetScalarIndex(v);
        }
        {
            static int v[6]{};
            static int r{};
            ImGui::DragScalarN(std::to_string(r).c_str(), ImGuiDataType_S32, v, IM_ARRAYSIZE(v));
            ImGui::SameLine();
            if (ImGui::Button("GetScalarIndexForExtent")) r = obj->GetScalarIndexForExtent(v);
        }
    }

    void vtkImageResliceMapper_setup(vtkImageResliceMapper* obj)
    {
        {
            const char* modeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
            if (int v = obj->GetSlabType(); ImGui::Combo("SlabType", &v, modeText, IM_ARRAYSIZE(modeText)))
            {
                obj->SetSlabType(v);
            }
            ImGui::Text("SlabTypeAsString: %s", obj->GetSlabTypeAsString());
        }
        if (float v = obj->GetSlabThickness(); ImGui::DragFloat("SlabThickness", &v, .01f, 0.f))
        {
            obj->SetSlabThickness(v);
        }
        if (bool v = obj->GetJumpToNearestSlice(); ImGui::Checkbox("JumpToNearestSlice", &v))
        {
            obj->SetJumpToNearestSlice(v);
        }
        if (bool v = obj->GetAutoAdjustImageQuality(); ImGui::Checkbox("AutoAdjustImageQuality", &v))
        {
            obj->SetAutoAdjustImageQuality(v);
        }
        if (bool v = obj->GetResampleToScreenPixels(); ImGui::Checkbox("ResampleToScreenPixels", &v))
        {
            obj->SetResampleToScreenPixels(v);
        }
        if (bool v = obj->GetSeparateWindowLevelOperation(); ImGui::Checkbox("SeparateWindowLevelOperation", &v))
        {
            obj->SetSeparateWindowLevelOperation(v);
        }
        if (int v = obj->GetSlabSampleFactor(); ImGui::DragInt("SlabSampleFactor", &v))
        {
            obj->SetSlabSampleFactor(v);
        }
        if (int v = obj->GetImageSampleFactor(); ImGui::DragInt("ImageSampleFactor", &v, 1, obj->GetImageSampleFactorMinValue(), obj->GetImageSampleFactorMaxValue()))
        {
            obj->SetImageSampleFactor(v);
        }
    }

    void vtkImageMapper3D_setup(vtkImageMapper3D* obj)
    {
        if (bool v = obj->GetBorder(); ImGui::Checkbox("Border", &v))
        {
            obj->SetBorder(v);
        }
        if (bool v = obj->GetBackground(); ImGui::Checkbox("Background", &v))
        {
            obj->SetBackground(v);
        }
        if (bool v = obj->GetStreaming(); ImGui::Checkbox("Streaming", &v))
        {
            obj->SetStreaming(v);
        }
        if (bool v = obj->GetSliceFacesCamera(); ImGui::Checkbox("SliceFacesCamera", &v))
        {
            obj->SetSliceFacesCamera(v);
        }
        if (bool v = obj->GetSliceAtFocalPoint(); ImGui::Checkbox("SliceAtFocalPoint", &v))
        {
            obj->SetSliceAtFocalPoint(v);
        }
        {
            double v[6];
            obj->GetIndexBounds(v);
            ImGui::Text(fmt::format("IndexBounds: {::.2f}", v).c_str());
        }

        vtkns::vtkObjSetup("SlicePlane", obj->GetSlicePlane(), ImGuiTreeNodeFlags_DefaultOpen);
    }

    void vtkImplicitFunction_setup(vtkImplicitFunction* obj)
    {

    }

    void vtkPlane_setup(vtkPlane* obj)
    {
        {
            ImGui::Text("Push:");
            ImGui::SameLine();
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##Push-", ImGuiDir_Left)) { obj->Push(-.5); }
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::ArrowButton("##Push+", ImGuiDir_Right)) { obj->Push(.5); }
            ImGui::PopButtonRepeat();
        }

        if (double v[3]; obj->GetOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
        {
            obj->SetOrigin(v);
        }

        if (double v[3]; obj->GetNormal(v), ImGui::DragScalarN("Normal", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
        {
            obj->SetNormal(v);
        }
    }

    void vtkImageSlice_setup(vtkImageSlice* obj)
    {
        ImGui::Text(fmt::format("MinXBound: {}", obj->GetMinXBound()).c_str());
        ImGui::Text(fmt::format("MaxXBound: {}", obj->GetMaxXBound()).c_str());
        ImGui::Text(fmt::format("MinYBound: {}", obj->GetMinYBound()).c_str());
        ImGui::Text(fmt::format("MaxYBound: {}", obj->GetMaxYBound()).c_str());
        ImGui::Text(fmt::format("MinZBound: {}", obj->GetMinZBound()).c_str());
        ImGui::Text(fmt::format("MaxZBound: {}", obj->GetMaxZBound()).c_str());
        if (bool v = obj->GetForceTranslucent(); ImGui::Checkbox("ForceTranslucent", &v))
        {
            obj->SetForceTranslucent(v);
        }
        vtkns::vtkObjSetup("Property", obj->GetProperty());
        vtkns::vtkObjSetup("Mapper", obj->GetMapper());
    }

    void vtkImageProperty_setup(vtkImageProperty* obj)
    {
        if (double v = obj->GetColorLevel(); ImGui::DragScalar("ColorLevel", ImGuiDataType_Double, &v))
        {
            obj->SetColorLevel(v);
        }
        if (double v = obj->GetColorWindow(); ImGui::DragScalar("ColorWindow", ImGuiDataType_Double, &v))
        {
            obj->SetColorWindow(v);
        }
        if (float v = obj->GetOpacity(); ImGui::DragFloat("Opacity", &v, .01, 0., 1.))
        {
            obj->SetOpacity(v);
        }
        if (int v = obj->GetLayerNumber(); ImGui::DragInt("LayerNumber", &v))
        {
            obj->SetLayerNumber(v);
        }
        if (bool v = obj->GetCheckerboard(); ImGui::Checkbox("Checkerboard", &v))
        {
            obj->SetCheckerboard(v);
        }
        if (double v[2]; obj->GetCheckerboardSpacing(v), ImGui::DragScalarN("CheckerboardSpacing", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01f))
        {
            obj->SetCheckerboardSpacing(v);
        }
        if (double v[2]; obj->GetCheckerboardOffset(v), ImGui::DragScalarN("CheckerboardOffset", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01f))
        {
            obj->SetCheckerboardOffset(v);
        }
        if (bool v = obj->GetBacking(); ImGui::Checkbox("Backing", &v))
        {
            obj->SetBacking(v);
        }
        if (float v[3] = { obj->GetBackingColor()[0],obj->GetBackingColor()[1],obj->GetBackingColor()[2] }; ImGui::ColorEdit3("BackingColor", v))
        {
            obj->SetBackingColor(v[0], v[1], v[2]);
        }
        if (float v = obj->GetAmbient(); ImGui::SliderFloat("Ambient", &v, 0., 1.))
        {
            obj->SetAmbient(v);
        }
        if (float v = obj->GetDiffuse(); ImGui::SliderFloat("Diffuse", &v, 0., 1.))
        {
            obj->SetDiffuse(v);
        }
        ImGui::Text(fmt::format("InterpolationType: {}", obj->GetInterpolationTypeAsString()).c_str());
        {
            const char* modeText[] = { "VTK_NEAREST_INTERPOLATION", "VTK_LINEAR_INTERPOLATION", "VTK_CUBIC_INTERPOLATION" };
            if (auto v = obj->GetInterpolationType(); ImGui::Combo("InterpolationType", &v, modeText, IM_ARRAYSIZE(modeText)))
            {
                obj->SetInterpolationType(v);
            }
        }
        vtkns::vtkObjSetup("LookupTable", obj->GetLookupTable());
    }

    void vtkImageActor_setup(vtkImageActor* obj)
    {
        if (bool v = obj->GetForceOpaque(); ImGui::Checkbox("ForceOpaque", &v))
        {
            obj->SetForceOpaque(v);
        }
        if (bool v = obj->GetInterpolate(); ImGui::Checkbox("Interpolate", &v))
        {
            obj->SetInterpolate(v);
        }
        if (float v = obj->GetOpacity(); ImGui::DragFloat("Opacity", &v, .01, obj->GetOpacityMinValue(), obj->GetOpacityMaxValue()))
        {
            obj->SetOpacity(v);
        }
        if (int v[6]; obj->GetDisplayExtent(v), ImGui::DragScalarN("DisplayExtent", ImGuiDataType_S32, v, std::size(v)))
        {
            obj->SetDisplayExtent(v);
        }
        if (double v[6]; obj->GetBounds(v), ImGui::DragScalarN("Bounds", ImGuiDataType_Double, v, std::size(v), 0.01f))
        {
            //pImageActor->SetBounds(v);
        }
        if (double v[6]; obj->GetDisplayBounds(v), ImGui::DragScalarN("DisplayBounds", ImGuiDataType_Double, v, std::size(v), 0.01f))
        {
            //pImageActor->SetBounds(v);
        }
        // 需要mapper是vtkImageSliceMapper
        ImGui::Text(fmt::format("SliceNumber: {}", obj->GetSliceNumber()).c_str());
        ImGui::Text(fmt::format("SliceNumberMin: {}", obj->GetSliceNumberMin()).c_str());
        ImGui::Text(fmt::format("SliceNumberMax: {}", obj->GetSliceNumberMax()).c_str());
        ImGui::Text(fmt::format("WholeZMin: {}", obj->GetWholeZMin()).c_str());
        ImGui::Text(fmt::format("WholeZMax: {}", obj->GetWholeZMax()).c_str());
        if (int v = obj->GetZSlice(); ImGui::DragInt("ZSlice", &v))
        {
            obj->SetZSlice(v);
        }
        vtkns::vtkObjSetup("Input", obj->GetInput());
    }

    void vtkImageFlip_setup(vtkImageFlip* obj)
    {
        if (bool v = obj->GetFlipAboutOrigin(); ImGui::Checkbox("FlipAboutOrigin", &v))
        {
            obj->SetFlipAboutOrigin(v);
        }
        if (bool v = obj->GetPreserveImageExtent(); ImGui::Checkbox("PreserveImageExtent", &v))
        {
            obj->SetPreserveImageExtent(v);
        }

        for (const auto i : { 0,1,2 })
        {
            if (ImGui::Button(std::to_string(i).c_str()))
            {
                obj->SetFilteredAxes(i);
                obj->Update();
            }
            ImGui::SameLine();
        }
        ImGui::Text(fmt::format("FilteredAxes: {}", obj->GetFilteredAxes()).c_str());
    }

    void vtk3DWidget_setup(vtk3DWidget* obj)
    {
        if (double v = obj->GetPlaceFactor(); ImGui::DragScalar("PlaceFactor", ImGuiDataType_Double, &v, 0.1f))
        {
            obj->SetPlaceFactor(v);
        }
        if (double v = obj->GetHandleSize(); ImGui::DragScalar("HandleSize", ImGuiDataType_Double, &v, 0.1f))
        {
            obj->SetHandleSize(v);
        }
    }

    void vtkPointWidget_setup(vtkPointWidget* obj)
    {
        if (bool v = obj->GetOutline(); ImGui::Checkbox("Outline", &v))
        {
            obj->SetOutline(v);
        }
        if (bool v = obj->GetXShadows(); ImGui::Checkbox("XShadows", &v))
        {
            obj->SetXShadows(v);
        }
        if (bool v = obj->GetYShadows(); ImGui::Checkbox("YShadows", &v))
        {
            obj->SetYShadows(v);
        }
        if (bool v = obj->GetZShadows(); ImGui::Checkbox("ZShadows", &v))
        {
            obj->SetZShadows(v);
        }
        if (bool v = obj->GetTranslationMode(); ImGui::Checkbox("TranslationMode", &v))
        {
            obj->SetTranslationMode(v);
        }
        if (ImGui::Button("AllOff")) obj->AllOff(); ImGui::SameLine(); if (ImGui::Button("AllOn")) obj->AllOn();
        if (double v[3]; obj->GetPosition(v), ImGui::DragScalarN("Position", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
        {
            obj->SetPosition(v);
        }
        if (double v = obj->GetHotSpotSize(); ImGui::DragScalar("HotSpotSize", ImGuiDataType_Double, &v, 0.1f))
        {
            obj->SetHotSpotSize(v);
        }
        vtkns::vtkObjSetup("Property", obj->GetProperty());
        vtkns::vtkObjSetup("SelectedProperty", obj->GetSelectedProperty());
    }

    void vtkWindow_setup(vtkWindow* obj)
    {

    }

    void vtkRenderWindow_setup(vtkRenderWindow* obj)
    {
        ImGui::Text(fmt::format("NumberOfLayers: {}", obj->GetNumberOfLayers()).c_str());
    }

    void vtkCaptionActor2D_setup(vtkCaptionActor2D* obj)
    {
        if (bool v = obj->GetBorder(); ImGui::Checkbox("Border", &v))
        {
            obj->SetBorder(v);
        }
        if (bool v = obj->GetAttachEdgeOnly(); ImGui::Checkbox("AttachEdgeOnly", &v))
        {
            obj->SetAttachEdgeOnly(v);
        }
        if (bool v = obj->GetLeader(); ImGui::Checkbox("Leader", &v))
        {
            obj->SetLeader(v);
        }
        if (bool v = obj->GetThreeDimensionalLeader(); ImGui::Checkbox("ThreeDimensionalLeader", &v))
        {
            obj->SetThreeDimensionalLeader(v);
        }
        if (float v[3]{ obj->GetAttachmentPoint()[0], obj->GetAttachmentPoint()[1], obj->GetAttachmentPoint()[2] }; ImGui::DragScalarN("AttachmentPoint", ImGuiDataType_Float, v, IM_ARRAYSIZE(v), 0.1f))
        {
            obj->SetAttachmentPoint(v[0], v[1], v[2]);
        }
        if (int v = obj->GetPadding(); ImGui::DragInt("Padding", &v))
        {
            obj->SetPadding(v);
        }
        if (float v = obj->GetLeaderGlyphSize(); ImGui::DragFloat("LeaderGlyphSize", &v))
        {
            obj->SetLeaderGlyphSize(v);
        }
    }

    void vtkCamera_setup(vtkCamera* obj)
    {
        constexpr auto myOff = 1;
        {
            double viewup[3];
            obj->GetViewUp(viewup);
            if (ImGui::DragScalarN("ViewUp", ImGuiDataType_Double, viewup, 3, 1.f))
            {
                obj->SetViewUp(viewup); // 函数内部会将viewup调整为单位向量
            }
        }
        {
            double pos[3];
            obj->GetPosition(pos);
            if (ImGui::DragScalarN("Position", ImGuiDataType_Double, pos, 3, 0.01f))
            {
                obj->SetPosition(pos);
            }
        }
        {
            double fp[3];
            obj->GetFocalPoint(fp);
            if (ImGui::DragScalarN("FocalPoint", ImGuiDataType_Double, fp, 3, 0.01f))
            {
                obj->SetFocalPoint(fp);
            }
        }

        if (int myroll = obj->GetRoll(); ImGui::DragInt("roll", &myroll, 1.f, -360, 360))
        {
            obj->SetRoll(myroll);
        }

        if (ImGui::TreeNode(u8"旋转"))
        {
            {
                {
                    ImGui::Text("Roll:");
                    ImGui::SameLine();
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Roll_left", ImGuiDir_Left)) { obj->Roll(-myOff); }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##Roll_right", ImGuiDir_Right)) { obj->Roll(myOff); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    ImGui::Text("%d", int(obj->GetRoll()));
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(Rotate the camera about the direction of projection.
This will spin the camera about its axis.
围绕投影方向旋转相机，这将使相机绕其轴旋转)");
                }
                {
                    ImGui::Text("Dolly:");
                    ImGui::SameLine();
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Dolly_left", ImGuiDir_Left)) { obj->Dolly(1.001); }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##Dolly_right", ImGuiDir_Right)) { obj->Dolly(0.999); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(Divide the camera's distance from the focal point by the given dolly value.
Use a value greater than one to dolly-in toward the focal point, and use a value less than one to dolly-out away from the focal point.
使用传入的参数修改position，使camera在投影方向上距离focalpoint变远或变近
效果就是camera的拉近拉远)");
                    //ImGui::SameLine();
                    //ImGui::Text("%d", int(camera->GetDolly()));
                }
                {
                    ImGui::Text("Elevation:");
                    ImGui::SameLine();
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Elevation_left", ImGuiDir_Left)) { obj->Elevation(-myOff); obj->OrthogonalizeViewUp(); /*不调用会错？？*/ }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##Elevation_right", ImGuiDir_Right)) { obj->Elevation(myOff); obj->OrthogonalizeViewUp(); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(Rotate the camera about the cross product of the negative of the direction of projection and the view up vector, using the focal point as the center of rotation.
The result is a vertical rotation of the scene.
不断修改viewup和position，使camera绕着focalpoint垂直转动。对比Pitch)");
                }
                ImGui::SameLine();
                {
                    ImGui::Text("Pitch:");
                    ImGui::SameLine();
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Pitch_left", ImGuiDir_Left)) { obj->Pitch(-myOff); obj->OrthogonalizeViewUp(); }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##Pitch_right", ImGuiDir_Right)) { obj->Pitch(myOff); obj->OrthogonalizeViewUp(); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(Rotate the focal point about the cross product of the view up vector and the direction of projection, using the camera's position as the center of rotation.
The result is a vertical rotation of the camera.
不断修改focalpoint和viewup使camera绕着position为中心（原地）垂直旋转。对比Elevation
)");
                }
                {
                    ImGui::Text("Azimuth:");
                    ImGui::SameLine();
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Azimuth_left", ImGuiDir_Left)) { obj->Azimuth(-myOff); }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##Azimuth_right", ImGuiDir_Right)) { obj->Azimuth(myOff); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(Rotate the camera about the view up vector centered at the focal point.
Note that the view up vector is whatever was set via SetViewUp, and is not necessarily perpendicular to the direction of projection.
The result is a horizontal rotation of the camera.
围绕以focalpoint为中心的viewup旋转相机（只有相机的position在变）
结果是相机的水平旋转
不断修改position使camera绕着focalpoint水平转动。对比Yaw)");
                    ImGui::SameLine();
                    {
                        static auto autoAzimuth = false;
                        ImGui::Checkbox("Auto", &autoAzimuth);
                        if (autoAzimuth)
                        {
                            obj->Azimuth(-myOff);
                        }
                    }
                }
                ImGui::SameLine();
                {
                    ImGui::Text("Yaw:");
                    ImGui::SameLine();
                    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::ArrowButton("##Yaw_left", ImGuiDir_Left)) { obj->Yaw(-myOff); }
                    ImGui::SameLine(0.0f, spacing);
                    if (ImGui::ArrowButton("##Yaw_right", ImGuiDir_Right)) { obj->Yaw(myOff); }
                    ImGui::PopButtonRepeat();
                    ImGui::SameLine();
                    vtkns::HelpMarker(u8R"(Rotate the focal point about the view up vector, using the camera's position as the center of rotation.
Note that the view up vector is whatever was set via SetViewUp, and is not necessarily perpendicular to the direction of projection.
The result is a horizontal rotation of the scene.
使用相机position作为旋转中心，围绕viewup旋转focalpoint（只有focalpoint在变）
结果是场景的水平旋转
不断修改focalpoint使camera绕着position为中心（原地）水平转动。对比Azimuth)");
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::Button("OrthogonalizeViewUp"))
        {
            obj->OrthogonalizeViewUp();
        }
        ImGui::SameLine();
        if (ImGui::Button("ComputeViewPlaneNormal"))
        {
            obj->ComputeViewPlaneNormal();
        }
        {
            static float near_, far_;
            double rangeVal[2];
            obj->GetClippingRange(rangeVal);
            near_ = rangeVal[0];
            far_ = rangeVal[1];
            if (ImGui::DragFloatRange2("ClippingRange", &near_, &far_, 0.1f, 0.0f, 10000.0f, "Near: %lf", "Far: %lf"))
            {
                rangeVal[0] = near_;
                rangeVal[1] = far_;
                obj->SetClippingRange(rangeVal);
            }
        }
        auto orientation = obj->GetOrientation();
        ImGui::Text("Orientation:[%lf,%lf,%lf,%lf]", orientation[0], orientation[1], orientation[2], orientation[3]);
        auto dop = obj->GetDirectionOfProjection(); // DirectionOfProjectio是通过FocalPoint减去Position再normalization得到的
        ImGui::Text("DirectionOfProjection:[%lf,%lf,%lf]", dop[0], dop[1], dop[2]);
#if 0
        {
            ImGui::Text("Distance:");
            ImGui::SameLine();
            float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##SetDistance_left", ImGuiDir_Left)) { pCamera->SetDistance(pCamera->GetDistance() - myOff); }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##SetDistance_right", ImGuiDir_Right)) { pCamera->SetDistance(pCamera->GetDistance() + myOff); }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            ImGui::Text("%lf", pCamera->GetDistance());
        }
#else
        if (float v = obj->GetDistance(); ImGui::SliderFloat("Distance", &v, -10., 10.))
        {
            obj->SetDistance(v); // 设置FocalPoint到camera的距离，会将FocalPoint的值调整为：Position + DirectionOfProjection * Distance
        }
#endif

#if 0
        {
            ImGui::Text("ViewAngle:");
            ImGui::SameLine();
            float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##SetViewAngle_left", ImGuiDir_Left)) { pCamera->SetViewAngle(pCamera->GetViewAngle() - myOff); }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##SetViewAngle_right", ImGuiDir_Right)) { pCamera->SetViewAngle(pCamera->GetViewAngle() + myOff); }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            ImGui::Text("%lf", pCamera->GetViewAngle());
        }
#else
        if (float v = obj->GetViewAngle(); ImGui::SliderFloat("ViewAngle", &v, 0.001, 180.))
        {
            obj->SetViewAngle(v);
        }
#endif
        if (float v = obj->GetParallelScale(); ImGui::SliderFloat("ParallelScale", &v, 0.001, 100))
        {
            obj->SetParallelScale(v);
        }
        if (bool v = obj->GetParallelProjection(); ImGui::Checkbox("ParallelProjection", &v))
        {
            obj->SetParallelProjection(v);
        }
        {
            ImGui::Text("Zoom:");
            ImGui::SameLine();
            float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##Zoom_left", ImGuiDir_Left)) { obj->Zoom(0.9); }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##Zoom_right", ImGuiDir_Right)) { obj->Zoom(1.1); }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            vtkns::HelpMarker(R"(In perspective mode, decrease the view angle by the specified factor.
In parallel mode, decrease the parallel scale by the specified factor.
A value greater than 1 is a zoom-in, a value less than 1 is a zoom-out.
@note This setting is ignored when UseExplicitProjectionTransformMatrix is true.)");
        }
        if (ImGui::Button(u8"水平翻转"))
        {
            obj->Azimuth(180.);
        }ImGui::SameLine();
        if (ImGui::Button(u8"垂直翻转"))
        {
            obj->Roll(180.);
            obj->Azimuth(180.);
        }ImGui::SameLine();
        if (ImGui::Button("Mirror"))
        {
            //double imageActorBound[6]{ 0 };
            //panData->getImageActor()->GetBounds(imageActorBound);
            //double* pCenter = panData->getImageActor()->GetCenter();
            //panData->getRenderer()->GetActiveCamera()->SetPosition(pCenter[0], pCenter[1], -1* panData->getRenderer()->GetActiveCamera()->GetDistance());
        }
        if (ImGui::Button(u8"旋转0"))
        {
            obj->SetViewUp(0.0, 1.0, 0.0);
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"旋转90"))
        {
            obj->SetViewUp(-1.0, 0.0, 0.0);
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"旋转180"))
        {
            obj->SetViewUp(0.0, -1.0, 0.0);
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"旋转270"))
        {
            obj->SetViewUp(1.0, 0.0, 0.0);
        }
    }

    void vtkProperty_setup(vtkProperty* obj)
    {
        {
            ImGui::Text("Representation"); ImGui::SameLine();
            int v = obj->GetRepresentation();
            ImGui::RadioButton("Points", &v, VTK_POINTS); ImGui::SameLine();
            ImGui::RadioButton("Wireframe", &v, VTK_WIREFRAME); ImGui::SameLine();
            ImGui::RadioButton("Surface", &v, VTK_SURFACE);
            if (obj->GetRepresentation() != v) // 这样写是因为ImGui::RadioButton在选中的情况下点击也会返回true
            {
                obj->SetRepresentation(v);
            }
        }

        if (bool lighting = obj->GetLighting(); ImGui::Checkbox("Lighting", &lighting))
        {
            obj->SetLighting(lighting);
        }
        if (bool f = obj->GetEdgeVisibility(); ImGui::Checkbox("EdgeVisibility", &f))
        {
            obj->SetEdgeVisibility(f);
        }
        if (bool f = obj->GetVertexVisibility(); ImGui::Checkbox("VertexVisibility", &f))
        {
            obj->SetVertexVisibility(f);
        }
        if (bool f = obj->GetRenderPointsAsSpheres(); ImGui::Checkbox("RenderPointsAsSpheres", &f))
        {
            obj->SetRenderPointsAsSpheres(f);
        }
        if (bool f = obj->GetRenderLinesAsTubes(); ImGui::Checkbox("RenderLinesAsTubes", &f))
        {
            obj->SetRenderLinesAsTubes(f);
        }
        float color[3] = { obj->GetColor()[0],obj->GetColor()[1],obj->GetColor()[2] };
        if (ImGui::ColorEdit3("Color", color))
        {
            obj->SetColor(color[0], color[1], color[2]);
        }
        float edgeColor[3] = { obj->GetEdgeColor()[0],obj->GetEdgeColor()[1],obj->GetEdgeColor()[2] };
        if (ImGui::ColorEdit3("EdgeColor", edgeColor))
        {
            obj->SetEdgeColor(edgeColor[0], edgeColor[1], edgeColor[2]);
        }
        float vertexColor[3] = { obj->GetVertexColor()[0],obj->GetVertexColor()[1],obj->GetVertexColor()[2] };
        if (ImGui::ColorEdit3("VertexColor", vertexColor))
        {
            obj->SetVertexColor(vertexColor[0], vertexColor[1], vertexColor[2]);
        }
        float coatColor[3] = { obj->GetCoatColor()[0],obj->GetCoatColor()[1],obj->GetCoatColor()[2] };
        if (ImGui::ColorEdit3("CoatColor", coatColor))
        {
            obj->SetCoatColor(coatColor[0], coatColor[1], coatColor[2]);
        }
        if (float opacity = obj->GetOpacity(); ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f, "opacity = %.3f"))
        {
            obj->SetOpacity(opacity);
        }
        if (float v = obj->GetLineWidth(); ImGui::SliderFloat("LineWidth", &v, 0.0f, 30.0f))
        {
            obj->SetLineWidth(v);
        }
        if (float v = obj->GetPointSize(); ImGui::SliderFloat("PointSize", &v, 1.0f, 100.0f))
        {
            obj->SetPointSize(v);
        }
        if (float v = obj->GetCoatStrength(); ImGui::SliderFloat("CoatStrength", &v, 0.0f, 1.0f))
        {
            obj->SetCoatStrength(v);
        }
        if (float v = obj->GetCoatRoughness(); ImGui::SliderFloat("CoatRoughness", &v, 0.0f, 1.0f))
        {
            obj->SetCoatRoughness(v);
        }
        if (float v = obj->GetMetallic(); ImGui::SliderFloat("Metallic", &v, 0.0f, 1.0f))
        {
            obj->SetMetallic(v);
        }
        if (int v = obj->GetLineStipplePattern(); ImGui::DragInt("LineStipplePattern", &v, 0xFF))
        {
            obj->SetLineStipplePattern(v);
        }
        if (int v = obj->GetLineStippleRepeatFactor(); ImGui::SliderInt("LineStippleRepeatFactor", &v, 1, 100))
        {
            obj->SetLineStippleRepeatFactor(v);
        }
    }
}

namespace vtkns
{
    void vtkObjSetup(std::string_view objName, vtkSmartPointer<vtkObject> vtkObj, const ImGuiTreeNodeFlags flags)
    {
        if (!vtkObj)
        {
            ImGui::Text("%s is nullptr", objName.data());
            return;
        }

        if (ImGui::TreeNodeEx(objName.data(), flags))
        {
            if (ImGui::CollapsingHeader("vtkObject", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Pointer: %0x", vtkObj.Get());
                ImGui::Text("MTime: %ld", vtkObj->GetMTime());
                ImGui::Text("Name: %s", vtkObj->GetClassName());

                // 继承自vtkObject
                if (const auto pProp = vtkProp::SafeDownCast(vtkObj); pProp && ImGui::CollapsingHeader("vtkProp", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (pProp->GetBounds())
                    {
                        ImGui::InputScalarN("Bounds", ImGuiDataType_Double, pProp->GetBounds(), 6, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    }
                    else
                    {
                        ImGui::Text("Bounds nullptr");
                    }

                    if (pProp->GetMatrix())
                    {
                        ImGui::InputScalarN("Matrix0", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 0, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                        ImGui::InputScalarN("Matrix1", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 4, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                        ImGui::InputScalarN("Matrix2", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 8, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                        ImGui::InputScalarN("Matrix3", ImGuiDataType_Double, pProp->GetMatrix()->GetData() + 12, 4, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    }
                    else
                    {
                        ImGui::Text("Matrix nullptr");
                    }

                    if (bool visibility = pProp->GetVisibility(); ImGui::Checkbox("Visibility ", &visibility))
                    {
                        pProp->SetVisibility(visibility);
                    }
                    ImGui::SameLine();
                    if (bool pickable = pProp->GetPickable(); ImGui::Checkbox("Pickable", &pickable))
                    {
                        pProp->SetPickable(pickable);
                    }
                    ImGui::SameLine();
                    if (bool dragable = pProp->GetDragable(); ImGui::Checkbox("Dragable", &dragable))
                    {
                        pProp->SetDragable(dragable);
                    }
                }
                else if (const auto pCamera = vtkCamera::SafeDownCast(vtkObj); pCamera && ImGui::CollapsingHeader("vtkCamera", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ::vtkCamera_setup(pCamera);
                }
                else if (const auto pViewport = vtkViewport::SafeDownCast(vtkObj); pViewport && ImGui::CollapsingHeader("vtkViewport", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (float v[3] = { pViewport->GetBackground()[0],pViewport->GetBackground()[1],pViewport->GetBackground()[2] }; ImGui::ColorEdit3("Background", v))
                    {
                        pViewport->SetBackground(v[0], v[1], v[2]);
                    }
                    // ?? 是否可以嵌套
                    if (const auto pRenderer = vtkRenderer::SafeDownCast(vtkObj); pRenderer && ImGui::CollapsingHeader("vtkRenderer", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (ImGui::Button("ResetCamera"))
                        {
                            pRenderer->ResetCamera();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("ResetCameraClippingRange"))
                        {
                            pRenderer->ResetCameraClippingRange();
                        }
                        if (bool v = pRenderer->GetUseDepthPeelingForVolumes(); ImGui::Checkbox("UseDepthPeelingForVolumes", &v))
                        {
                            pRenderer->SetUseDepthPeelingForVolumes(v);
                        }
                        if (int v = pRenderer->GetMaximumNumberOfPeels(); ImGui::SliderInt("MaximumNumberOfPeels", &v, 0, 100))
                        {
                            pRenderer->SetMaximumNumberOfPeels(v);
                        }
                        if (float v = pRenderer->GetOcclusionRatio(); ImGui::SliderFloat("OcclusionRatio", &v, 0., 10.))
                        {
                            pRenderer->SetOcclusionRatio(v);
                        }
                    }
                }
                else if (const auto pDataObject = vtkDataObject::SafeDownCast(vtkObj); pDataObject && ImGui::CollapsingHeader("vtkDataObject", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkDataObject_setup(pDataObject);
                    if (const auto pDataSet = vtkDataSet::SafeDownCast(vtkObj); pDataSet && ImGui::CollapsingHeader("vtkDataSet", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        vtkDataSet_setup(pDataSet);
                        if (const auto pImageData = vtkImageData::SafeDownCast(vtkObj); pImageData && ImGui::CollapsingHeader("vtkImageData", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            vtkImageData_setup(pImageData);
                        }
                        else if (const auto pPointSet = vtkPointSet::SafeDownCast(vtkObj); pPointSet && ImGui::CollapsingHeader("vtkPointSet", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ::vtkPointSet_setup(pPointSet);
                        }
                    }
                }
                else if (const auto pAbstractPicker = vtkAbstractPicker::SafeDownCast(vtkObj); pAbstractPicker && ImGui::TreeNodeEx("vtkAbstractPicker", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    {
                        double v[3];
                        pAbstractPicker->GetPickPosition(v);
                        ImGui::Text(fmt::format("PickPosition: {::.2f}", v).c_str());
                    }
                    {
                        double v[3];
                        pAbstractPicker->GetSelectionPoint(v);
                        ImGui::Text(fmt::format("SelectionPoint: {::.2f}", v).c_str());
                    }

                    if (const auto pAbstractPropPicker = vtkAbstractPropPicker::SafeDownCast(vtkObj); pAbstractPropPicker && ImGui::TreeNodeEx("vtkAbstractPropPicker", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        vtkObjSetup("ViewProp", pAbstractPropPicker->GetViewProp());
                        ImGui::Text(fmt::format("ViewProp: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetViewProp())).c_str());
                        ImGui::Text(fmt::format("Prop3D: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetProp3D())).c_str());
                        ImGui::Text(fmt::format("Actor2D: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetActor2D())).c_str());
                        ImGui::Text(fmt::format("Actor: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetActor())).c_str());
                        ImGui::Text(fmt::format("Volume: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetVolume())).c_str());
                        ImGui::Text(fmt::format("Assembly: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetAssembly())).c_str());
                        ImGui::Text(fmt::format("PropAssembly: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetPropAssembly())).c_str());
                        ImGui::Text(fmt::format("Path: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetPath())).c_str());

                        if (const auto pPicker = vtkPicker::SafeDownCast(vtkObj); pPicker && ImGui::TreeNodeEx("vtkPicker", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (double v = pPicker->GetTolerance(); ImGui::DragScalar("Tolerance", ImGuiDataType_Double, &v, 0.01f))
                            {
                                pPicker->SetTolerance(v);
                            }
                            ImGui::Text(fmt::format("Actors-NumberOfItems: {}", pPicker->GetActors()->GetNumberOfItems()).c_str());
                            ImGui::Text(fmt::format("Prop3Ds-NumberOfItems: {}", pPicker->GetProp3Ds()->GetNumberOfItems()).c_str());

                            if (const auto pPointPicker = vtkPointPicker::SafeDownCast(vtkObj); pPointPicker && ImGui::TreeNodeEx("vtkPointPicker", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                ImGui::Text(fmt::format("PointId: {}", pPointPicker->GetPointId()).c_str());
                                ImGui::TreePop();
                            }
                            else if (const auto pCellPicker = vtkCellPicker::SafeDownCast(vtkObj); pCellPicker && ImGui::TreeNodeEx("vtkCellPicker", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                ImGui::Text(fmt::format("PointId: {}", pCellPicker->GetPointId()).c_str());
                                ImGui::Text(fmt::format("CellId: {}", pCellPicker->GetCellId()).c_str());
                                ImGui::Text(fmt::format("SubId: {}", pCellPicker->GetSubId()).c_str());
                                {
                                    double v[3];
                                    pCellPicker->GetPickNormal(v);
                                    ImGui::Text(fmt::format("PickNormal: {}", v).c_str());
                                }
                                {
                                    int v[3];
                                    pCellPicker->GetPointIJK(v);
                                    ImGui::Text(fmt::format("PointIJK: {}", v).c_str());
                                }
                                {
                                    int v[3];
                                    pCellPicker->GetCellIJK(v);
                                    ImGui::Text(fmt::format("CellIJK: {}", v).c_str());
                                }
                                {
                                    double v[3];
                                    pCellPicker->GetPCoords(v);
                                    ImGui::Text(fmt::format("PCoords: {::.2f}", v).c_str());
                                }
                                ImGui::TreePop();
                            }

                            ImGui::TreePop();
                        }
                        else if (const auto pPropPicker = vtkPropPicker::SafeDownCast(vtkObj); pPropPicker && ImGui::TreeNodeEx("vtkPropPicker", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::TreePop();
                        }

                        ImGui::TreePop();
                    }
                    else if (const auto pWorldPointPicker = vtkWorldPointPicker::SafeDownCast(vtkObj); pWorldPointPicker && ImGui::TreeNodeEx("vtkWorldPointPicker", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
                else if (const auto pProperty = vtkProperty::SafeDownCast(vtkObj); pProperty && ImGui::TreeNodeEx("vtkProperty", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ::vtkProperty_setup(pProperty);
                    ImGui::TreePop();
                }
                else if (const auto pImageViewer2 = vtkImageViewer2::SafeDownCast(vtkObj); pImageViewer2 && ImGui::TreeNodeEx("vtkImageViewer2", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (int v[3]; pImageViewer2->GetSliceRange(v), v[2] = pImageViewer2->GetSlice(), ImGui::SliderInt("Slice", &v[2], v[0], v[1]))
                    {
                        pImageViewer2->SetSlice(v[2]);
                    }

                    if (float v = pImageViewer2->GetColorLevel(); ImGui::SliderFloat("ColorLevel", &v, 0., 10000.))
                    {
                        pImageViewer2->SetColorLevel(v);
                    }

                    if (float v = pImageViewer2->GetColorWindow(); ImGui::SliderFloat("ColorWindow", &v, 0., 10000.))
                    {
                        pImageViewer2->SetColorWindow(v);
                    }

                    if (int v[2]{ pImageViewer2->GetSize()[0], pImageViewer2->GetSize()[1] }; ImGui::DragInt2("Size", v))
                    {
                        pImageViewer2->SetSize(v);
                    }

                    if (int v[2]{ pImageViewer2->GetPosition()[0], pImageViewer2->GetPosition()[1] }; ImGui::DragInt2("Position", v))
                    {
                        pImageViewer2->SetPosition(v);
                    }

                    {
                        const char* modeText[] = { "SLICE_ORIENTATION_YZ", "SLICE_ORIENTATION_XZ", "SLICE_ORIENTATION_XY" };
                        if (auto v = pImageViewer2->GetSliceOrientation(); ImGui::Combo("SliceOrientation", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pImageViewer2->SetSliceOrientation(v);
                        }
                    }

                    if (bool v = pImageViewer2->GetOffScreenRendering(); ImGui::Checkbox("OffScreenRendering ", &v))
                    {
                        pImageViewer2->SetOffScreenRendering(v);
                    }

                    vtkns::vtkObjSetup("ImageActor", pImageViewer2->GetImageActor(), ImGuiTreeNodeFlags_DefaultOpen);

                    if (const auto pResliceImageViewer = vtkResliceImageViewer::SafeDownCast(vtkObj); pResliceImageViewer && ImGui::TreeNodeEx("vtkResliceImageViewer", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (bool v = pResliceImageViewer->GetSliceScrollOnMouseWheel(); ImGui::Checkbox("SliceScrollOnMouseWheel", &v))
                        {
                            pResliceImageViewer->SetSliceScrollOnMouseWheel(v);
                        }

                        if (bool v = pResliceImageViewer->GetThickMode(); ImGui::Checkbox("ThickMode ", &v))
                        {
                            pResliceImageViewer->SetThickMode(v);
                        }

                        {
                            const char* modeText[] = { "RESLICE_AXIS_ALIGNED", "RESLICE_OBLIQUE" };
                            if (auto v = pResliceImageViewer->GetResliceMode(); ImGui::Combo("ResliceMode", &v, modeText, IM_ARRAYSIZE(modeText)))
                            {
                                pResliceImageViewer->SetResliceMode(v);
                            }
                        }

                        if (float v = pResliceImageViewer->GetSliceScrollFactor(); ImGui::SliderFloat("SliceScrollFactor", &v, 0., 10.))
                        {
                            pResliceImageViewer->SetSliceScrollFactor(v);
                        }

                        vtkObjSetup("ResliceCursor", pResliceImageViewer->GetResliceCursor(), ImGuiTreeNodeFlags_DefaultOpen);
                        vtkObjSetup("ResliceCursorWidget", pResliceImageViewer->GetResliceCursorWidget(), ImGuiTreeNodeFlags_DefaultOpen);

                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
                else if (const auto pResliceCursor = vtkResliceCursor::SafeDownCast(vtkObj); pResliceCursor && ImGui::TreeNodeEx("vtkResliceCursor", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (bool v = pResliceCursor->GetThickMode(); ImGui::Checkbox("ThickMode ", &v))
                    {
                        pResliceCursor->SetThickMode(v);
                    }

                    if (double v[3]; pResliceCursor->GetThickness(v), ImGui::DragScalarN("Thickness", ImGuiDataType_Double, v, 3, 0.5f))
                    {
                        pResliceCursor->SetThickness(v);
                    }
                    ImGui::TreePop();
                }
                else if (const auto pInteractorObserver = vtkInteractorObserver::SafeDownCast(vtkObj); pInteractorObserver && ImGui::CollapsingHeader("vtkInteractorObserver", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (bool v = pInteractorObserver->GetEnabled(); ImGui::Checkbox("Enabled ", &v))
                    {
                        pInteractorObserver->SetEnabled(v);
                    }

                    if (ImGui::Button("On")) pInteractorObserver->On(); ImGui::SameLine();
                    if (ImGui::Button("Off")) pInteractorObserver->Off();

                    if (const auto pAbstractWidget = vtkAbstractWidget::SafeDownCast(vtkObj); pAbstractWidget && ImGui::CollapsingHeader("vtkAbstractWidget", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (bool v = pAbstractWidget->GetManagesCursor(); ImGui::Checkbox("ManagesCursor ", &v))
                        {
                            pAbstractWidget->SetManagesCursor(v);
                        }
                        if (bool v = pAbstractWidget->GetProcessEvents(); ImGui::Checkbox("ProcessEvents ", &v))
                        {
                            pAbstractWidget->SetProcessEvents(v);
                        }
                        vtkObjSetup("Representation", pAbstractWidget->GetRepresentation(), ImGuiTreeNodeFlags_DefaultOpen);
                        if (const auto pResliceCursorWidget = vtkResliceCursorWidget::SafeDownCast(vtkObj); pResliceCursorWidget && ImGui::TreeNodeEx("vtkResliceCursorWidget", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::TreePop();
                        }
                        else if (const auto pBoxWidget2 = vtkBoxWidget2::SafeDownCast(vtkObj); pBoxWidget2 && ImGui::TreeNodeEx("vtkBoxWidget2", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (bool v = pBoxWidget2->GetTranslationEnabled(); ImGui::Checkbox("TranslationEnabled ", &v))
                            {
                                pBoxWidget2->SetTranslationEnabled(v);
                            }
                            if (bool v = pBoxWidget2->GetRotationEnabled(); ImGui::Checkbox("RotationEnabled ", &v))
                            {
                                pBoxWidget2->SetRotationEnabled(v);
                            }
                            if (bool v = pBoxWidget2->GetScalingEnabled(); ImGui::Checkbox("ScalingEnabled ", &v))
                            {
                                pBoxWidget2->SetScalingEnabled(v);
                            }
                            if (bool v = pBoxWidget2->GetMoveFacesEnabled(); ImGui::Checkbox("MoveFacesEnabled ", &v))
                            {
                                pBoxWidget2->SetMoveFacesEnabled(v);
                            }
                            ImGui::TreePop();
                        }
                        else if (const auto pLineWidget2 = vtkLineWidget2::SafeDownCast(vtkObj); pLineWidget2 && ImGui::CollapsingHeader("vtkLineWidget2", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            //vtkObjSetup("LineRepresentation", pLineWidget2->GetLineRepresentation(), ImGuiTreeNodeFlags_DefaultOpen);
                        }
                        else if (const auto pDistanceWidget = vtkDistanceWidget::SafeDownCast(vtkObj); pDistanceWidget && ImGui::CollapsingHeader("vtkDistanceWidget", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            {
                                const char* modeText[] = { "Start", "Define", "Manipulate" };
                                ImGui::Text(fmt::format("WidgetState: {}", modeText[pDistanceWidget->GetWidgetState()]).c_str());
                            }
                        }
                        else if (const auto pBorderWidget = vtkBorderWidget::SafeDownCast(vtkObj); pBorderWidget && ImGui::CollapsingHeader("vtkBorderWidget", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (bool v = pBorderWidget->GetResizable(); ImGui::Checkbox("Resizable ", &v))
                            {
                                pBorderWidget->SetResizable(v);
                            }
                            if (bool v = pBorderWidget->GetSelectable(); ImGui::Checkbox("Selectable ", &v))
                            {
                                pBorderWidget->SetSelectable(v);
                            }
                        }
                    }
                    else if (const auto p3DWidget = vtk3DWidget::SafeDownCast(vtkObj); p3DWidget && ImGui::CollapsingHeader("p3DWidget", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        vtk3DWidget_setup(p3DWidget);
                    }
                }
                else if (const auto pAlgorithm = vtkAlgorithm::SafeDownCast(vtkObj); pAlgorithm && ImGui::CollapsingHeader("vtkAlgorithm", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::Button("Update"))
                    {
                        pAlgorithm->Update();
                    }

                    if (ImGui::Button("UpdateInformation"))
                    {
                        pAlgorithm->UpdateInformation();
                    }
                    if (const auto pImageAlgorithm = vtkImageAlgorithm::SafeDownCast(vtkObj); pImageAlgorithm && ImGui::TreeNodeEx("vtkImageAlgorithm", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (const auto pThreadedImageAlgorithm = vtkThreadedImageAlgorithm::SafeDownCast(vtkObj); pThreadedImageAlgorithm && ImGui::TreeNodeEx("vtkThreadedImageAlgorithm", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (const auto pImageSlab = vtkImageSlab::SafeDownCast(vtkObj); pImageSlab && ImGui::TreeNodeEx("vtkImageSlab", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                {
                                    const char* modeText[] = { "VTK_IMAGE_SLAB_MIN", "VTK_IMAGE_SLAB_MAX", "VTK_IMAGE_SLAB_MEAN", "VTK_IMAGE_SLAB_SUM" };
                                    if (auto v = pImageSlab->GetOperation(); ImGui::Combo("Operation", &v, modeText, IM_ARRAYSIZE(modeText)))
                                    {
                                        pImageSlab->SetOperation(v);
                                    }
                                }
                                {
                                    const char* modeText[] = { "X", "Y", "Z" };
                                    if (auto v = pImageSlab->GetOrientation(); ImGui::Combo("Orientation", &v, modeText, IM_ARRAYSIZE(modeText)))
                                    {
                                        pImageSlab->SetOrientation(v);
                                    }
                                }
                                if (int v[2]; pImageSlab->GetSliceRange(v), ImGui::DragInt2("SliceRange", v))
                                {
                                    pImageSlab->SetSliceRange(v);
                                }

                                ImGui::TreePop();
                            }
                            else if (const auto pImageThreshold = vtkImageThreshold::SafeDownCast(vtkObj); pImageThreshold && ImGui::CollapsingHeader("vtkImageThreshold", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                ImGui::Text("OutputScalarType: %d", pImageThreshold->GetOutputScalarType());
                                if (bool v = pImageThreshold->GetReplaceIn(); ImGui::Checkbox("ReplaceIn ", &v))
                                {
                                    pImageThreshold->SetReplaceIn(v);
                                }
                                if (bool v = pImageThreshold->GetReplaceOut(); ImGui::Checkbox("ReplaceOut ", &v))
                                {
                                    pImageThreshold->SetReplaceOut(v);
                                }
                                if (float v = pImageThreshold->GetInValue(); ImGui::DragFloat("InValue", &v))
                                {
                                    pImageThreshold->SetInValue(v);
                                }
                                if (float v = pImageThreshold->GetOutValue(); ImGui::DragFloat("OutValue", &v))
                                {
                                    pImageThreshold->SetOutValue(v);
                                }
                                {
                                    float l = pImageThreshold->GetLowerThreshold();
                                    float u = pImageThreshold->GetUpperThreshold();
                                    if (ImGui::DragFloatRange2("ThresholdBetween", &l, &u))
                                    {
                                        pImageThreshold->ThresholdBetween(l, u);
                                    }
                                    if (ImGui::DragFloat("ThresholdByLower", &u))
                                    {
                                        pImageThreshold->ThresholdByLower(u);
                                    }
                                    if (ImGui::DragFloat("ThresholdByUpper", &l))
                                    {
                                        pImageThreshold->ThresholdByUpper(l);
                                    }
                                }
                            }
                            else if (const auto pImageReslice = vtkImageReslice::SafeDownCast(vtkObj); pImageReslice && ImGui::CollapsingHeader("vtkImageReslice", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                ::vtkImageReslice_setup(pImageReslice);
                            }
                            ImGui::TreePop();
                        }
                        else if (const auto pExtractVOI = vtkExtractVOI::SafeDownCast(vtkObj); pExtractVOI && ImGui::CollapsingHeader("vtkExtractVOI", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (int v[6]; pExtractVOI->GetVOI(v), ImGui::DragScalarN("VOI", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
                            {
                                pExtractVOI->SetVOI(v);
                                pExtractVOI->Update();
                            }
                        }
                        ImGui::TreePop();
                    }
                    else if (const auto pPolyDataAlgorithm = vtkPolyDataAlgorithm::SafeDownCast(vtkObj); pPolyDataAlgorithm && ImGui::TreeNodeEx("vtkPolyDataAlgorithm", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        vtkObjSetup("Output", pPolyDataAlgorithm->GetOutput());

                        if (const auto pLineSource = vtkLineSource::SafeDownCast(vtkObj); pLineSource && ImGui::TreeNodeEx("vtkLineSource", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (double v[3]; pLineSource->GetPoint1(v), ImGui::DragScalarN("Point1", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
                            {
                                pLineSource->SetPoint1(v);
                            }
                            if (double v[3]; pLineSource->GetPoint2(v), ImGui::DragScalarN("Point2", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
                            {
                                pLineSource->SetPoint2(v);
                            }
                            if (bool v = pLineSource->GetUseRegularRefinement(); ImGui::Checkbox("UseRegularRefinement", &v))
                            {
                                pLineSource->SetUseRegularRefinement(v);
                            }
                            if (int v = pLineSource->GetResolution(); ImGui::DragInt("Resolution", &v))
                            {
                                pLineSource->SetResolution(v);
                            }
                            if (int v = pLineSource->GetOutputPointsPrecision(); ImGui::DragInt("OutputPointsPrecision", &v))
                            {
                                pLineSource->SetOutputPointsPrecision(v);
                            }
                            if (pLineSource->GetPoints())
                            {
                                ImGui::Text("PointsNumber: %d", pLineSource->GetPoints()->GetNumberOfPoints());
                            }

                            ImGui::TreePop();
                        }
                        else if (const auto pVolumeOutlineSource = vtkVolumeOutlineSource::SafeDownCast(vtkObj); pVolumeOutlineSource && ImGui::TreeNodeEx("vtkVolumeOutlineSource", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (bool v = pVolumeOutlineSource->GetGenerateFaces(); ImGui::Checkbox("GenerateFaces", &v))
                            {
                                pVolumeOutlineSource->SetGenerateFaces(v);
                            }
                            if (bool v = pVolumeOutlineSource->GetGenerateScalars(); ImGui::Checkbox("GenerateScalars", &v))
                            {
                                pVolumeOutlineSource->SetGenerateScalars(v);
                            }
                            if (bool v = pVolumeOutlineSource->GetGenerateOutline(); ImGui::Checkbox("GenerateOutline", &v))
                            {
                                pVolumeOutlineSource->SetGenerateOutline(v);
                            }
                            if (float v[3] = { pVolumeOutlineSource->GetColor()[0],pVolumeOutlineSource->GetColor()[1],pVolumeOutlineSource->GetColor()[2] }; ImGui::ColorEdit3("Color", v))
                            {
                                pVolumeOutlineSource->SetColor(v[0], v[1], v[2]);
                            }
                            if (float v[3]{ pVolumeOutlineSource->GetActivePlaneColor()[0],pVolumeOutlineSource->GetActivePlaneColor()[1],pVolumeOutlineSource->GetActivePlaneColor()[2] }; ImGui::ColorEdit3("ActivePlaneColor", v))
                            {
                                pVolumeOutlineSource->SetActivePlaneColor(v[0], v[1], v[2]);
                            }
                            if (int v = pVolumeOutlineSource->GetActivePlaneId(); ImGui::SliderInt("ActivePlaneId", &v, -1, 5))
                            {
                                pVolumeOutlineSource->SetActivePlaneId(v);
                            }
                            ImGui::TreePop();
                        }
                        else if (const auto pFlyingEdges3D = vtkFlyingEdges3D::SafeDownCast(vtkObj); pFlyingEdges3D && ImGui::CollapsingHeader("vtkFlyingEdges3D", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ::vtkFlyingEdges3D_setup(pFlyingEdges3D);
                        }
                        else if (const auto pMarchingCubes = vtkMarchingCubes::SafeDownCast(vtkObj); pMarchingCubes && ImGui::CollapsingHeader("vtkMarchingCubes", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ::vtkMarchingCubes_setup(pMarchingCubes);
                        }
                        else if (const auto pTransformPolyDataFilter = vtkTransformPolyDataFilter::SafeDownCast(vtkObj); pTransformPolyDataFilter && ImGui::CollapsingHeader("vtkTransformPolyDataFilter", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ::vtkTransformPolyDataFilter_setup(pTransformPolyDataFilter);
                        }
                        else if (const auto pPlaneSource = vtkPlaneSource::SafeDownCast(vtkObj); pPlaneSource && ImGui::CollapsingHeader("vtkPlaneSource", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ::vtkPlaneSource_setup(pPlaneSource);
                        }
                        else if (const auto pImageToPolyDataFilter = vtkImageToPolyDataFilter::SafeDownCast(vtkObj); pImageToPolyDataFilter && ImGui::CollapsingHeader("vtkImageToPolyDataFilter", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            vtkImageToPolyDataFilter_setup(pImageToPolyDataFilter);
                        }
                        ImGui::TreePop();
                    }
                    else if (const auto pAbstractMapper = vtkAbstractMapper::SafeDownCast(vtkObj); pAbstractMapper && ImGui::CollapsingHeader("vtkAbstractMapper", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Text(fmt::format("NumberOfClippingPlanes: {}", pAbstractMapper->GetNumberOfClippingPlanes()).c_str());
                        if (const auto pAbstractMapper3D = vtkAbstractMapper3D::SafeDownCast(vtkObj); pAbstractMapper3D && ImGui::CollapsingHeader("vtkAbstractMapper3D", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            {
                                double v[6];
                                pAbstractMapper3D->GetBounds(v);
                                ImGui::Text(fmt::format("Bounds: {::.2f}", v).c_str());
                            }
                            {
                                double v[3];
                                pAbstractMapper3D->GetCenter(v);
                                ImGui::Text(fmt::format("Center: {::.2f}", v).c_str());
                            }
                            ImGui::Text(fmt::format("Length: {}", pAbstractMapper3D->GetLength()).c_str());
                            ImGui::Text(fmt::format("IsARayCastMapper: {}", pAbstractMapper3D->IsARayCastMapper()).c_str());
                            ImGui::Text(fmt::format("IsARenderIntoImageMapper: {}", pAbstractMapper3D->IsARenderIntoImageMapper()).c_str());
                        }
                    }
                    else if (const auto pDataSetAlgorithm = vtkDataSetAlgorithm::SafeDownCast(vtkObj); pDataSetAlgorithm && ImGui::CollapsingHeader("vtkDataSetAlgorithm", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        vtkDataSetAlgorithm_setup(pDataSetAlgorithm);
                    }
                }
                else if (const auto pTextProperty = vtkTextProperty::SafeDownCast(vtkObj); pTextProperty && ImGui::CollapsingHeader("vtkTextProperty", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (float v[3] = { pTextProperty->GetColor()[0],pTextProperty->GetColor()[1],pTextProperty->GetColor()[2] }; ImGui::ColorEdit3("Color", v))
                    {
                        pTextProperty->SetColor(v[0], v[1], v[2]);
                    }
                    if (double v = pTextProperty->GetOpacity(); ImGui::DragScalar("Opacity", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pTextProperty->SetOpacity(v);
                    }
                    if (float v[3] = { pTextProperty->GetBackgroundColor()[0],pTextProperty->GetBackgroundColor()[1],pTextProperty->GetBackgroundColor()[2] }; ImGui::ColorEdit3("BackgroundColor", v))
                    {
                        pTextProperty->SetBackgroundColor(v[0], v[1], v[2]);
                    }
                    if (double v = pTextProperty->GetBackgroundOpacity(); ImGui::DragScalar("BackgroundOpacity", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pTextProperty->SetBackgroundOpacity(v);
                    }
                    {
                        double vRGBA[4];
                        pTextProperty->GetBackgroundRGBA(vRGBA);
                        if (float v[4] = { vRGBA[0],vRGBA[1],vRGBA[2],vRGBA[3] }; ImGui::ColorEdit4("BackgroundRGBA", v))
                        {
                            pTextProperty->SetBackgroundRGBA(v[0], v[1], v[2], v[3]);
                        }
                    }
                    if (float v[3] = { pTextProperty->GetFrameColor()[0],pTextProperty->GetFrameColor()[1],pTextProperty->GetFrameColor()[2] }; ImGui::ColorEdit3("FrameColor", v))
                    {
                        pTextProperty->SetFrameColor(v[0], v[1], v[2]);
                    }
                    if (int v = pTextProperty->GetFrameWidth(); ImGui::DragInt("FrameWidth", &v))
                    {
                        pTextProperty->SetFrameWidth(v);
                    }
                    if (bool v = pTextProperty->GetFrame(); ImGui::Checkbox("Frame ", &v))
                    {
                        pTextProperty->SetFrame(v);
                    }
                    if (bool v = pTextProperty->GetBold(); ImGui::Checkbox("Bold ", &v))
                    {
                        pTextProperty->SetBold(v);
                    }
                    ImGui::SameLine();
                    if (bool v = pTextProperty->GetItalic(); ImGui::Checkbox("Italic ", &v))
                    {
                        pTextProperty->SetItalic(v);
                    }
                    if (bool v = pTextProperty->GetInteriorLinesVisibility(); ImGui::Checkbox("InteriorLinesVisibility ", &v))
                    {
                        pTextProperty->SetInteriorLinesVisibility(v);
                    }
                    if (int v = pTextProperty->GetInteriorLinesWidth(); ImGui::DragInt("InteriorLinesWidth", &v))
                    {
                        pTextProperty->SetInteriorLinesWidth(v);
                    }
                    if (float v[3] = { pTextProperty->GetInteriorLinesColor()[0],pTextProperty->GetInteriorLinesColor()[1],pTextProperty->GetInteriorLinesColor()[2] }; ImGui::ColorEdit3("InteriorLinesColor", v))
                    {
                        pTextProperty->SetInteriorLinesColor(v[0], v[1], v[2]);
                    }
                    if (double v = pTextProperty->GetCellOffset(); ImGui::DragScalar("CellOffset", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pTextProperty->SetCellOffset(v);
                    }
                    if (float v = pTextProperty->GetFontSize(); ImGui::SliderFloat("FontSize", &v, 0, 100))
                    {
                        pTextProperty->SetFontSize(v);
                    }
                    if (ImGui::Button("SetFontFamilyToArial")) pTextProperty->SetFontFamilyToArial();
                    if (ImGui::Button("SetFontFamilyToCourier")) pTextProperty->SetFontFamilyToCourier();
                    if (ImGui::Button("SetFontFamilyToTimes")) pTextProperty->SetFontFamilyToTimes();
                }
                else if (const auto pProperty2D = vtkProperty2D::SafeDownCast(vtkObj); pProperty2D && ImGui::CollapsingHeader("vtkProperty2D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (float v[3] = { pProperty2D->GetColor()[0],pProperty2D->GetColor()[1],pProperty2D->GetColor()[2] }; ImGui::ColorEdit3("Color", v))
                    {
                        pProperty2D->SetColor(v[0], v[1], v[2]);
                    }
                    if (double v = pProperty2D->GetOpacity(); ImGui::DragScalar("Opacity", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pProperty2D->SetOpacity(v);
                    }
                    if (float v = pProperty2D->GetPointSize(); ImGui::DragFloat("PointSize", &v, 0.01f))
                    {
                        pProperty2D->SetPointSize(v);
                    }
                    if (float v = pProperty2D->GetLineWidth(); ImGui::DragFloat("LineWidth", &v, 0.01f))
                    {
                        pProperty2D->SetLineWidth(v);
                    }
                    if (int v = pProperty2D->GetLineStipplePattern(); ImGui::DragInt("LineStipplePattern", &v))
                    {
                        pProperty2D->SetLineStipplePattern(v);
                    }
                    if (int v = pProperty2D->GetLineStippleRepeatFactor(); ImGui::DragInt("LineStippleRepeatFactor", &v))
                    {
                        pProperty2D->SetLineStippleRepeatFactor(v);
                    }
                    {
                        const char* modeText[] = { "VTK_BACKGROUND_LOCATION", "VTK_FOREGROUND_LOCATION" };
                        if (auto v = pProperty2D->GetDisplayLocation(); ImGui::Combo("DisplayLocation", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pProperty2D->SetDisplayLocation(v);
                        }
                    }
                }
                else if (const auto pImplicitFunction = vtkImplicitFunction::SafeDownCast(vtkObj); pImplicitFunction && ImGui::CollapsingHeader("vtkImplicitFunction", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkImplicitFunction_setup(pImplicitFunction);
                }
                else if (const auto pImageProperty = vtkImageProperty::SafeDownCast(vtkObj); pImageProperty && ImGui::CollapsingHeader("vtkImageProperty", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkImageProperty_setup(pImageProperty);
                }
                else if (const auto pWindow = vtkWindow::SafeDownCast(vtkObj); pWindow && ImGui::CollapsingHeader("vtkWindow", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkWindow_setup(pWindow);
                }
                else if (const auto pAbstractImageInterpolator = vtkAbstractImageInterpolator::SafeDownCast(vtkObj); pAbstractImageInterpolator && ImGui::CollapsingHeader("vtkAbstractImageInterpolator", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkAbstractImageInterpolator_setup(pAbstractImageInterpolator);
                }
                else if (const auto pMatrix4x4 = vtkMatrix4x4::SafeDownCast(vtkObj); pMatrix4x4 && ImGui::CollapsingHeader("vtkMatrix4x4", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkMatrix4x4_setup(pMatrix4x4);
                }

                // vtkWindow
                if (const auto pRenderWindow = vtkRenderWindow::SafeDownCast(vtkObj); pRenderWindow && ImGui::CollapsingHeader("vtkRenderWindow", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkRenderWindow_setup(pRenderWindow);
                }

                // vtkDataSetAlgorithm
                if (const auto pElevationFilter = vtkElevationFilter::SafeDownCast(vtkObj); pElevationFilter && ImGui::CollapsingHeader("vtkElevationFilter", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkElevationFilter_setup(pElevationFilter);
                }

                // vtkAbstractMapper3D
                if (const auto pMapper = vtkMapper::SafeDownCast(vtkObj); pMapper && ImGui::CollapsingHeader("vtkMapper", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (const auto pPolyDataMapper = vtkPolyDataMapper::SafeDownCast(vtkObj); pPolyDataMapper && ImGui::CollapsingHeader("vtkPolyDataMapper", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::InputScalarN("Bounds", ImGuiDataType_Double, pPolyDataMapper->GetBounds(), 6, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    }
                }
                else if (const auto pAbstractVolumeMapper = vtkAbstractVolumeMapper::SafeDownCast(vtkObj); pAbstractVolumeMapper && ImGui::CollapsingHeader("vtkAbstractVolumeMapper", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (const auto pVolumeMapper = vtkVolumeMapper::SafeDownCast(vtkObj); pVolumeMapper && ImGui::CollapsingHeader("vtkVolumeMapper", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        // BlendMode
                        {
                            const char* modeText[] = { "COMPOSITE_BLEND", "MAXIMUM_INTENSITY_BLEND", "MINIMUM_INTENSITY_BLEND", "AVERAGE_INTENSITY_BLEND", "ADDITIVE_BLEND", "ISOSURFACE_BLEND", "SLICE_BLEND" };
                            if (auto v = pVolumeMapper->GetBlendMode(); ImGui::Combo("BlendMode", &v, modeText, IM_ARRAYSIZE(modeText)))
                            {
                                pVolumeMapper->SetBlendMode(v);
                            }
                        }
#if 0
                        {
                            const char* items[] = { "VTK_CROP_SUBVOLUME", "VTK_CROP_FENCE", "VTK_CROP_INVERTED_FENCE", "VTK_CROP_CROSS", "VTK_CROP_INVERTED_CROSS" };
                            int dataArray[] = { VTK_CROP_SUBVOLUME, VTK_CROP_FENCE, VTK_CROP_INVERTED_FENCE, VTK_CROP_CROSS, VTK_CROP_INVERTED_CROSS };
                            static int currentIdx = -1;
                            if (ImGui::Combo("CroppingRegionFlags", &currentIdx, items, IM_ARRAYSIZE(items)))
                            {
                                const auto n = dataArray[currentIdx] / ::spacing[2];
                                ::reslice->SetSlabNumberOfSlices(n);
                                ::colorMap->Update();
                            }
                        }
#else
                        {
                            std::map<int, char*> myMap
                            {
                                {VTK_CROP_SUBVOLUME,"VTK_CROP_SUBVOLUME"},
                                {VTK_CROP_FENCE,"VTK_CROP_FENCE"},
                                {VTK_CROP_INVERTED_FENCE,"VTK_CROP_INVERTED_FENCE"},
                                {VTK_CROP_CROSS,"VTK_CROP_CROSS"},
                                {VTK_CROP_INVERTED_CROSS,"VTK_CROP_INVERTED_CROSS"}
                            };
                            for (auto i : myMap)
                            {
                                if (ImGui::Button(i.second)) pVolumeMapper->SetCroppingRegionFlags(i.first); ImGui::SameLine();
                            }
                            ImGui::Text(myMap[pVolumeMapper->GetCroppingRegionFlags()]);
                        }
#endif

                        if (bool v = pVolumeMapper->GetCropping(); ImGui::Checkbox("Cropping", &v))
                        {
                            pVolumeMapper->SetCropping(v);
                        }
                        if (double v[6]; pVolumeMapper->GetCroppingRegionPlanes(v), ImGui::DragScalarN("CroppingRegionPlanes", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
                        {
                            pVolumeMapper->SetCroppingRegionPlanes(v);
                        }
                        if (const auto pGPUVolumeRayCastMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(vtkObj); pGPUVolumeRayCastMapper && ImGui::TreeNodeEx("vtkGPUVolumeRayCastMapper", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            if (float v = pGPUVolumeRayCastMapper->GetImageSampleDistance(); ImGui::SliderFloat("ImageSampleDistance", &v, 0., 10.))
                            {
                                pGPUVolumeRayCastMapper->SetImageSampleDistance(v);
                            }
                            if (float v = pGPUVolumeRayCastMapper->GetSampleDistance(); ImGui::SliderFloat("SampleDistance", &v, 0.01, 3.)) // 调到0.001会崩溃且变卡
                            {
                                pGPUVolumeRayCastMapper->SetSampleDistance(v);
                            }
                            if (bool v = pGPUVolumeRayCastMapper->GetAutoAdjustSampleDistances(); ImGui::Checkbox("AutoAdjustSampleDistances", &v))
                            {
                                pGPUVolumeRayCastMapper->SetAutoAdjustSampleDistances(v);
                            }
                            if (bool v = pGPUVolumeRayCastMapper->GetUseJittering(); ImGui::Checkbox("UseJittering", &v))
                            {
                                pGPUVolumeRayCastMapper->SetUseJittering(v);
                            }
                            ImGui::TreePop();
                        }
                    }
                }
                else if (const auto pImageMapper3D = vtkImageMapper3D::SafeDownCast(vtkObj); pImageMapper3D && ImGui::CollapsingHeader("vtkImageMapper3D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkImageMapper3D_setup(pImageMapper3D);
                }

                // vtkImplicitFunction
                if (const auto pPlane = vtkPlane::SafeDownCast(vtkObj); pPlane && ImGui::CollapsingHeader("vtkPlane", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkPlane_setup(pPlane);
                }

                // vtk3DWidget
                if (const auto pPointWidget = vtkPointWidget::SafeDownCast(vtkObj); pPointWidget && ImGui::CollapsingHeader("vtkPointWidget", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkPointWidget_setup(pPointWidget);
                }

                // 继承自vtkProp
                if (const auto pProp3D = vtkProp3D::SafeDownCast(vtkObj); pProp3D && ImGui::CollapsingHeader("vtkProp3D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    double pos[3];
                    pProp3D->GetPosition(pos);
                    if (ImGui::DragScalarN("Position", ImGuiDataType_Double, pos, 3, 0.1f))
                    {
                        pProp3D->SetPosition(pos);
                    }

                    double origin[3];
                    pProp3D->GetOrigin(origin);
                    if (ImGui::DragScalarN("Origin", ImGuiDataType_Double, origin, 3, 0.1f))
                    {
                        pProp3D->SetOrigin(origin);
                    }

                    double scale[3];
                    pProp3D->GetScale(scale);
                    if (ImGui::DragScalarN("Scale", ImGuiDataType_Double, scale, 3, 0.001f))
                    {
                        pProp3D->SetScale(scale);
                    }

                    ImGui::InputScalarN("Center", ImGuiDataType_Double, pProp3D->GetCenter(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    ImGui::InputScalarN("XRange", ImGuiDataType_Double, pProp3D->GetXRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    ImGui::InputScalarN("YRange", ImGuiDataType_Double, pProp3D->GetYRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    ImGui::InputScalarN("ZRange", ImGuiDataType_Double, pProp3D->GetZRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    ImGui::InputScalarN("Orientation", ImGuiDataType_Double, pProp3D->GetOrientation(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    ImGui::Text("IsIdentity: %s", pProp3D->GetIsIdentity() ? "true" : "false");

                    {
                        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                        const auto n = 5;

                        ImGui::Text("RotateX:");
                        ImGui::SameLine();
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##leftRotateX", ImGuiDir_Left)) { pProp3D->RotateX(-n); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##rightRotateX", ImGuiDir_Right)) { pProp3D->RotateX(n); }
                        ImGui::PopButtonRepeat();

                        ImGui::Text("RotateY:");
                        ImGui::SameLine();
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##leftRotateY", ImGuiDir_Left)) { pProp3D->RotateY(-n); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##rightRotateY", ImGuiDir_Right)) { pProp3D->RotateY(n); }
                        ImGui::PopButtonRepeat();

                        ImGui::Text("RotateZ:");
                        ImGui::SameLine();
                        ImGui::PushButtonRepeat(true);
                        if (ImGui::ArrowButton("##leftRotateZ", ImGuiDir_Left)) { pProp3D->RotateZ(-n); }
                        ImGui::SameLine(0.0f, spacing);
                        if (ImGui::ArrowButton("##rightRotateZ", ImGuiDir_Right)) { pProp3D->RotateZ(n); }
                        ImGui::PopButtonRepeat();
                    }
                }
                else if (const auto pWidgetRepresentation = vtkWidgetRepresentation::SafeDownCast(vtkObj); pWidgetRepresentation && ImGui::CollapsingHeader("vtkWidgetRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (bool v = pWidgetRepresentation->GetPickingManaged(); ImGui::Checkbox("PickingManaged", &v))
                    {
                        pWidgetRepresentation->SetPickingManaged(v);
                    }
                    if (float v = pWidgetRepresentation->GetPlaceFactor(); ImGui::SliderFloat("PlaceFactor", &v, 0., 2.))
                    {
                        pWidgetRepresentation->SetPlaceFactor(v);
                    }
                    if (float v = pWidgetRepresentation->GetHandleSize(); ImGui::SliderFloat("HandleSize", &v, 0., 20.))
                    {
                        pWidgetRepresentation->SetHandleSize(v);
                    }
                    ImGui::Text(fmt::format("InteractionState: {}", pWidgetRepresentation->GetInteractionState()).c_str());
                }
                else if (const auto pActor2D = vtkActor2D::SafeDownCast(vtkObj); pActor2D && ImGui::CollapsingHeader("vtkActor2D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (float v = pActor2D->GetWidth(); ImGui::SliderFloat("Width", &v, 0, 100))
                    {
                        pActor2D->SetWidth(v);
                    }
                    if (float v = pActor2D->GetHeight(); ImGui::SliderFloat("Height", &v, 0, 100))
                    {
                        pActor2D->SetHeight(v);
                    }
                    if (float v[2]{ pActor2D->GetPosition()[0], pActor2D->GetPosition()[1] }; ImGui::DragFloat2("Position", v, 0.01f))
                    {
                        pActor2D->SetPosition(v[0], v[1]);
                    }
                    if (float v[2]{ pActor2D->GetPosition2()[0], pActor2D->GetPosition2()[1] }; ImGui::DragFloat2("Position2", v, 0.01f))
                    {
                        pActor2D->SetPosition2(v[0], v[1]);
                    }
                }

                // 继承自vtkActor2D
                if (const auto pAxisActor2D = vtkAxisActor2D::SafeDownCast(vtkObj); pAxisActor2D && ImGui::CollapsingHeader("vtkAxisActor2D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text(fmt::format("Title: {}", pAxisActor2D->GetTitle()).c_str());
                    if (float v[2]{ pAxisActor2D->GetPoint1()[0], pAxisActor2D->GetPoint1()[1] }; ImGui::DragFloat2("Point1", v, 0.01f))
                    {
                        pAxisActor2D->SetPoint1(v[0], v[1]);
                    }
                    if (float v[2]{ pAxisActor2D->GetPoint2()[0], pAxisActor2D->GetPoint2()[1] }; ImGui::DragFloat2("Point2", v, 0.01f))
                    {
                        pAxisActor2D->SetPoint2(v[0], v[1]);
                    }
                    if (bool v = pAxisActor2D->GetRulerMode(); ImGui::Checkbox("RulerMode ", &v))
                    {
                        pAxisActor2D->SetRulerMode(v);
                    }
                    if (bool v = pAxisActor2D->GetAdjustLabels(); ImGui::Checkbox("AdjustLabels ", &v))
                    {
                        pAxisActor2D->SetAdjustLabels(v);
                    }
                    if (bool v = pAxisActor2D->GetAxisVisibility(); ImGui::Checkbox("AxisVisibility ", &v))
                    {
                        pAxisActor2D->SetAxisVisibility(v);
                    }
                    if (bool v = pAxisActor2D->GetTickVisibility(); ImGui::Checkbox("TickVisibility ", &v))
                    {
                        pAxisActor2D->SetTickVisibility(v);
                    }
                    if (bool v = pAxisActor2D->GetLabelVisibility(); ImGui::Checkbox("LabelVisibility ", &v))
                    {
                        pAxisActor2D->SetLabelVisibility(v);
                    }
                    if (bool v = pAxisActor2D->GetTitleVisibility(); ImGui::Checkbox("TitleVisibility ", &v))
                    {
                        pAxisActor2D->SetTitleVisibility(v);
                    }
                    if (float v = pAxisActor2D->GetRulerDistance(); ImGui::SliderFloat("RulerDistance", &v, 0., 20.))
                    {
                        pAxisActor2D->SetRulerDistance(v);
                    }
                    if (int v = pAxisActor2D->GetNumberOfLabels(); ImGui::DragInt("NumberOfLabels", &v))
                    {
                        pAxisActor2D->SetNumberOfLabels(v);
                    }
                    if (int v = pAxisActor2D->GetTickOffset(); ImGui::DragInt("TickOffset", &v))
                    {
                        pAxisActor2D->SetTickOffset(v);
                    }
                    if (int v = pAxisActor2D->GetTickLength(); ImGui::DragInt("TickLength", &v))
                    {
                        pAxisActor2D->SetTickLength(v);
                    }
                    if (bool v = pAxisActor2D->GetUseFontSizeFromProperty(); ImGui::Checkbox("UseFontSizeFromProperty ", &v))
                    {
                        pAxisActor2D->SetUseFontSizeFromProperty(v);
                    }
                    if (double v = pAxisActor2D->GetFontFactor(); ImGui::DragScalar("FontFactor", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pAxisActor2D->SetFontFactor(v); // 需要手动刷新？？
                    }
                    vtkObjSetup("TitleTextProperty", pAxisActor2D->GetTitleTextProperty());
                }
                else if (const auto pCaptionActor2D = vtkCaptionActor2D::SafeDownCast(vtkObj); pCaptionActor2D && ImGui::CollapsingHeader("vtkCaptionActor2D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkCaptionActor2D_setup(pCaptionActor2D);
                }

                // 继承自vtkWidgetRepresentation
                if (const auto pBoxRepresentation = vtkBoxRepresentation::SafeDownCast(vtkObj); pBoxRepresentation && ImGui::CollapsingHeader("vtkBoxRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (bool v = pBoxRepresentation->GetInsideOut(); ImGui::Checkbox("InsideOut", &v))
                    {
                        pBoxRepresentation->SetInsideOut(v);
                    }
                    if (bool v = pBoxRepresentation->GetOutlineCursorWires(); ImGui::Checkbox("OutlineCursorWires", &v))
                    {
                        pBoxRepresentation->SetOutlineCursorWires(v);
                    }
                    if (bool v = pBoxRepresentation->GetOutlineFaceWires(); ImGui::Checkbox("OutlineFaceWires", &v))
                    {
                        pBoxRepresentation->SetOutlineFaceWires(v);
                    }
                    if (bool v = pBoxRepresentation->GetTwoPlaneMode(); ImGui::Checkbox("TwoPlaneMode", &v))
                    {
                        pBoxRepresentation->SetTwoPlaneMode(v);
                    }
                    if (bool v = pBoxRepresentation->GetSnapToAxes(); ImGui::Checkbox("SnapToAxes", &v))
                    {
                        pBoxRepresentation->SetSnapToAxes(v);
                    }
                    if (ImGui::Button("HandlesOn")) pBoxRepresentation->HandlesOn();
                    ImGui::SameLine();
                    if (ImGui::Button("HandlesOff")) pBoxRepresentation->HandlesOff();
                    if (ImGui::Button("StepBackward")) pBoxRepresentation->StepBackward();
                    ImGui::SameLine();
                    if (ImGui::Button("StepForward")) pBoxRepresentation->StepForward();
                }
                else if (const auto pLineRepresentation = vtkLineRepresentation::SafeDownCast(vtkObj); pLineRepresentation && ImGui::CollapsingHeader("vtkLineRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (bool v = pLineRepresentation->GetDistanceAnnotationVisibility(); ImGui::Checkbox("DistanceAnnotationVisibility ", &v))
                    {
                        pLineRepresentation->SetDistanceAnnotationVisibility(v);
                    }
                    if (bool v = pLineRepresentation->GetDirectionalLine(); ImGui::Checkbox("DirectionalLine ", &v))
                    {
                        pLineRepresentation->SetDirectionalLine(v);
                    }
                    ImGui::Text(pLineRepresentation->GetDistanceAnnotationFormat());
                    ImGui::Text(fmt::format("Distance: {}", pLineRepresentation->GetDistance()).c_str());
                    if (float v[3]{ pLineRepresentation->GetDistanceAnnotationScale()[0], pLineRepresentation->GetDistanceAnnotationScale()[1],pLineRepresentation->GetDistanceAnnotationScale()[2] }; ImGui::DragFloat3("DistanceAnnotationScale", v, .1f))
                    {
                        pLineRepresentation->SetDistanceAnnotationScale(v[0], v[1], v[2]);
                    }
                    {
                        const char* modeText[] = { "Outside", "OnP1", "OnP2", "TranslatingP1", "TranslatingP2", "OnLine", "Scaling" };
                        ImGui::Text(fmt::format("InteractionState: {}", modeText[pLineRepresentation->GetInteractionState()]).c_str());
                    }
                    vtkObjSetup("EndPointProperty", pLineRepresentation->GetEndPointProperty());
                    vtkObjSetup("EndPoint2Property", pLineRepresentation->GetEndPoint2Property());
                    vtkObjSetup("LineProperty", pLineRepresentation->GetLineProperty());
                    vtkObjSetup("SelectedLineProperty", pLineRepresentation->GetSelectedLineProperty());
                    vtkObjSetup("DistanceAnnotationProperty", pLineRepresentation->GetDistanceAnnotationProperty());
                }
                else if (const auto pDistanceRepresentation = vtkDistanceRepresentation::SafeDownCast(vtkObj); pDistanceRepresentation && ImGui::CollapsingHeader("vtkDistanceRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    {
                        const char* modeText[] = { "Outside", "NearP1", "NearP2" };
                        ImGui::Text(fmt::format("InteractionState: {}", modeText[pDistanceRepresentation->GetInteractionState()]).c_str());
                    }
                    ImGui::Text(fmt::format("Distance: {}", pDistanceRepresentation->GetDistance()).c_str());
                    if (bool v = pDistanceRepresentation->GetRulerMode(); ImGui::Checkbox("RulerMode ", &v))
                    {
                        pDistanceRepresentation->SetRulerMode(v);
                    }
                    if (float v = pDistanceRepresentation->GetRulerDistance(); ImGui::SliderFloat("RulerDistance", &v, 0., 20.))
                    {
                        pDistanceRepresentation->SetRulerDistance(v);
                    }
                    if (int v = pDistanceRepresentation->GetNumberOfRulerTicks(); ImGui::SliderInt("NumberOfRulerTicks", &v, 0., 20.))
                    {
                        pDistanceRepresentation->SetNumberOfRulerTicks(v);
                    }
                    if (float v = pDistanceRepresentation->GetScale(); ImGui::SliderFloat("Scale", &v, 0., 10.))
                    {
                        pDistanceRepresentation->SetScale(v);
                    }
                    if (double v[3]; pDistanceRepresentation->GetPoint1WorldPosition(v), ImGui::DragScalarN("Point1WorldPosition", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.0001f))
                    {
                        pDistanceRepresentation->SetPoint1WorldPosition(v);
                    }
                    if (double v[3]; pDistanceRepresentation->GetPoint2WorldPosition(v), ImGui::DragScalarN("Point2WorldPosition", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.0001f))
                    {
                        pDistanceRepresentation->SetPoint2WorldPosition(v);
                    }
                    if (double v[3]; pDistanceRepresentation->GetPoint1DisplayPosition(v), ImGui::DragScalarN("Point1DisplayPosition", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
                    {
                        pDistanceRepresentation->SetPoint1DisplayPosition(v);
                    }
                    if (double v[3]; pDistanceRepresentation->GetPoint2DisplayPosition(v), ImGui::DragScalarN("Point2DisplayPosition", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
                    {
                        pDistanceRepresentation->SetPoint2DisplayPosition(v);
                    }
                    if (int v = pDistanceRepresentation->GetTolerance(); ImGui::DragInt("Tolerance", &v))
                    {
                        pDistanceRepresentation->SetTolerance(v);
                    }
                    vtkObjSetup("Point1Representation", pDistanceRepresentation->GetPoint1Representation());
                    vtkObjSetup("Point2Representation", pDistanceRepresentation->GetPoint2Representation());
                }
                else if (const auto pBorderRepresentation = vtkBorderRepresentation::SafeDownCast(vtkObj); pBorderRepresentation && ImGui::CollapsingHeader("vtkBorderRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (float v[2]{ pBorderRepresentation->GetPosition()[0], pBorderRepresentation->GetPosition()[1] }; ImGui::DragFloat2("Position", v, 0.01f))
                    {
                        pBorderRepresentation->SetPosition(v[0], v[1]);
                    }
                    if (float v[2]{ pBorderRepresentation->GetPosition2()[0], pBorderRepresentation->GetPosition2()[1] }; ImGui::DragFloat2("Position2", v, 0.01f))
                    {
                        pBorderRepresentation->SetPosition2(v[0], v[1]);
                    }
                    if (float v[3] = { pBorderRepresentation->GetBorderColor()[0],pBorderRepresentation->GetBorderColor()[1],pBorderRepresentation->GetBorderColor()[2] }; ImGui::ColorEdit3("BorderColor", v))
                    {
                        pBorderRepresentation->SetBorderColor(v[0], v[1], v[2]);
                    }
                    if (float v[3] = { pBorderRepresentation->GetPolygonColor()[0],pBorderRepresentation->GetPolygonColor()[1],pBorderRepresentation->GetPolygonColor()[2] }; ImGui::ColorEdit3("PolygonColor", v))
                    {
                        pBorderRepresentation->SetPolygonColor(v[0], v[1], v[2]);
                    }
                    if (double v = pBorderRepresentation->GetPolygonOpacity(); ImGui::DragScalar("PolygonOpacity", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pBorderRepresentation->SetPolygonOpacity(v);
                    }
                    {
                        double vPolygonRGBA[4];
                        pBorderRepresentation->GetPolygonRGBA(vPolygonRGBA);
                        if (float v[4] = { vPolygonRGBA[0],vPolygonRGBA[1],vPolygonRGBA[2],vPolygonRGBA[3] }; ImGui::ColorEdit4("PolygonRGBA", v))
                        {
                            pBorderRepresentation->SetPolygonRGBA(v[0], v[1], v[2], v[3]);
                        }
                    }
                    if (double v = pBorderRepresentation->GetCornerRadiusStrength(); ImGui::DragScalar("CornerRadiusStrength", ImGuiDataType_Double, &v, 0.01f))
                    {
                        pBorderRepresentation->SetCornerRadiusStrength(v);
                    }
                    if (int v = pBorderRepresentation->GetCornerResolution(); ImGui::DragInt("CornerResolution", &v))
                    {
                        pBorderRepresentation->SetCornerResolution(v);
                    }
                    if (float v = pBorderRepresentation->GetBorderThickness(); ImGui::DragFloat("BorderThickness", &v, 0.01f))
                    {
                        pBorderRepresentation->SetBorderThickness(v);
                    }
                    if (int v = pBorderRepresentation->GetTolerance(); ImGui::DragInt("Tolerance", &v))
                    {
                        pBorderRepresentation->SetTolerance(v);
                    }
                    {
                        const char* modeText[] = { "AnyLocation", "LowerLeftCorner", "LowerRightCorner", "LowerCenter", "UpperLeftCorner", "UpperRightCorner", "UpperCenter" };
                        if (auto v = pBorderRepresentation->GetWindowLocation(); ImGui::Combo("WindowLocation", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pBorderRepresentation->SetWindowLocation(v);
                        }
                    }
                    {
                        double v[2];
                        ImGui::Text(fmt::format("Size: {}", (pBorderRepresentation->GetSize(v), v)).c_str()); // ??
                    }
                    ImGui::Text(fmt::format("ShowBorderMinValue: {}", pBorderRepresentation->GetShowBorderMinValue()).c_str());
                    ImGui::Text(fmt::format("ShowBorderMaxValue: {}", pBorderRepresentation->GetShowBorderMaxValue()).c_str());
                    ImGui::Text(fmt::format("Moving: {}", pBorderRepresentation->GetMoving()).c_str());
                    {
                        const char* modeText[] = { "BORDER_OFF", "BORDER_ON", "BORDER_ACTIVE" };
                        if (auto v = pBorderRepresentation->GetShowBorder(); ImGui::Combo("ShowBorder", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pBorderRepresentation->SetShowBorder(v);
                        }
                    }
                    {
                        const char* modeText[] = { "BORDER_OFF", "BORDER_ON", "BORDER_ACTIVE" };
                        if (auto v = pBorderRepresentation->GetShowVerticalBorder(); ImGui::Combo("ShowVerticalBorder", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pBorderRepresentation->SetShowVerticalBorder(v);
                        }
                    }
                    {
                        const char* modeText[] = { "BORDER_OFF", "BORDER_ON", "BORDER_ACTIVE" };
                        if (auto v = pBorderRepresentation->GetShowHorizontalBorder(); ImGui::Combo("ShowHorizontalBorder", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pBorderRepresentation->SetShowHorizontalBorder(v);
                        }
                    }
                    {
                        const char* modeText[] = { "BORDER_OFF", "BORDER_ON", "BORDER_ACTIVE" };
                        if (auto v = pBorderRepresentation->GetShowPolygonBackground(); ImGui::Combo("ShowPolygonBackground", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pBorderRepresentation->SetShowPolygonBackground(v);
                        }
                    }
                    {
                        const char* modeText[] = { "BORDER_OFF", "BORDER_ON", "BORDER_ACTIVE" };
                        if (auto v = pBorderRepresentation->GetShowPolygon(); ImGui::Combo("ShowPolygon", &v, modeText, IM_ARRAYSIZE(modeText)))
                        {
                            pBorderRepresentation->SetShowPolygon(v);
                        }
                    }
                }
                else if (const auto pHandleRepresentation = vtkHandleRepresentation::SafeDownCast(vtkObj); pHandleRepresentation && ImGui::CollapsingHeader("vtkHandleRepresentation", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    {
                        char* v[] = { "Outside","Nearby", "Selecting", "Translating", "Scaling" };
                        ImGui::Text(fmt::format("InteractionState: {}", v[pHandleRepresentation->GetInteractionState()]).c_str());
                    }
                    {
                        double dv[3];
                        if (float v[3]; pHandleRepresentation->GetDisplayPosition(dv), v[0] = dv[0], v[1] = dv[1], v[2] = dv[2], ImGui::DragFloat3("DisplayPosition", v, 0.1f))
                        {
                            dv[0] = v[0];
                            dv[1] = v[1];
                            dv[2] = v[2];
                            pHandleRepresentation->SetDisplayPosition(dv);
                        }
                    }
                    {
                        double dv[3];
                        if (float v[3]; pHandleRepresentation->GetWorldPosition(dv), v[0] = dv[0], v[1] = dv[1], v[2] = dv[2], ImGui::DragFloat3("WorldPosition", v, 0.1f))
                        {
                            dv[0] = v[0];
                            dv[1] = v[1];
                            dv[2] = v[2];
                            pHandleRepresentation->SetWorldPosition(dv);
                        }
                    }
                    if (int v = pHandleRepresentation->GetTolerance(); ImGui::DragInt("Tolerance", &v))
                    {
                        pHandleRepresentation->SetTolerance(v);
                    }
                    if (bool v = pHandleRepresentation->GetActiveRepresentation(); ImGui::Checkbox("ActiveRepresentation", &v))
                    {
                        pHandleRepresentation->SetActiveRepresentation(v);
                    }
                    if (bool v = pHandleRepresentation->GetConstrained(); ImGui::Checkbox("Constrained", &v))
                    {
                        pHandleRepresentation->SetConstrained(v);
                    }
                    {
                        double v[2]{ 0.1, 0.1 };
                        if (ImGui::Button("Translate")) pHandleRepresentation->Translate(v);
                    }
                }

                // 继承自vtkHandleRepresentation
                if (const auto pPointHandleRepresentation2D = vtkPointHandleRepresentation2D::SafeDownCast(vtkObj); pPointHandleRepresentation2D && ImGui::CollapsingHeader("vtkPointHandleRepresentation2D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::Button("Highlight")) pPointHandleRepresentation2D->Highlight(1);
                }

                // 继承自vtkDistanceRepresentation
                if (const auto pDistanceRepresentation2D = vtkDistanceRepresentation2D::SafeDownCast(vtkObj); pDistanceRepresentation2D && ImGui::CollapsingHeader("vtkDistanceRepresentation2D", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkObjSetup("Axis", pDistanceRepresentation2D->GetAxis());
                    vtkObjSetup("AxisProperty", pDistanceRepresentation2D->GetAxisProperty());
                }
                else if (const auto pDistanceRepresentation3D = vtkDistanceRepresentation3D::SafeDownCast(vtkObj); pDistanceRepresentation3D && ImGui::CollapsingHeader("vtkDistanceRepresentation3D", ImGuiTreeNodeFlags_DefaultOpen))
                {

                }

                // vtkImageReslice
                if (const auto pImageFlip = vtkImageFlip::SafeDownCast(vtkObj); pImageFlip && ImGui::CollapsingHeader("vtkImageFlip", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkImageFlip_setup(pImageFlip);
                }

                // vtkDataObject
#if 0
                if (const auto pDataSet = vtkDataSet::SafeDownCast(vtkObj); pDataSet && ImGui::CollapsingHeader("vtkDataSet", ImGuiTreeNodeFlags_DefaultOpen))
                {

                }
#endif

                // vtkImageMapper3D
                if (const auto pImageSliceMapper = vtkImageSliceMapper::SafeDownCast(vtkObj); pImageSliceMapper && ImGui::CollapsingHeader("vtkImageSliceMapper", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text("Orientation: %d", pImageSliceMapper->GetOrientation()); ImGui::SameLine();
                    if (ImGui::Button("OX")) pImageSliceMapper->SetOrientationToX(); ImGui::SameLine();
                    if (ImGui::Button("OY")) pImageSliceMapper->SetOrientationToY(); ImGui::SameLine();
                    if (ImGui::Button("OZ")) pImageSliceMapper->SetOrientationToZ(); ImGui::SameLine();
                    if (ImGui::Button("OI")) pImageSliceMapper->SetOrientationToI(); ImGui::SameLine();
                    if (ImGui::Button("OJ")) pImageSliceMapper->SetOrientationToJ(); ImGui::SameLine();
                    if (ImGui::Button("OK")) pImageSliceMapper->SetOrientationToK();
                    ImGui::Text("SliceNumberMinValue: %d", pImageSliceMapper->GetSliceNumberMinValue());
                    ImGui::Text("SliceNumberMaxValue: %d", pImageSliceMapper->GetSliceNumberMaxValue());
                    {
                        double v[6];
                        pImageSliceMapper->GetIndexBounds(v);
                        ImGui::InputScalarN("IndexBounds", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
                    }
                    if (int v = pImageSliceMapper->GetSliceNumber(); ImGui::DragInt("SliceNumber", &v))
                    {
                        pImageSliceMapper->SetSliceNumber(v);
                    }
                    //if (int v[6]; pImageSliceMapper->GetDisplayExtent(v), ImGui::DragScalarN("DisplayExtent", ImGuiDataType_S32, v, std::size(v)))
                    //{
                    //    pImageSliceMapper->SetDisplayExtent(v);
                    //}
                    if (double v[6]; pImageSliceMapper->GetBounds(v), ImGui::DragScalarN("Bounds", ImGuiDataType_Double, v, std::size(v), 0.01f))
                    {
                        //pImageActor->SetBounds(v);
                    }
                    if (bool v = pImageSliceMapper->GetCropping(); ImGui::Checkbox("Cropping", &v))
                    {
                        pImageSliceMapper->SetCropping(v);
                    }
                    if (int v[6]; pImageSliceMapper->GetCroppingRegion(v), ImGui::DragScalarN("CroppingRegion", ImGuiDataType_S32, v, std::size(v)))
                    {
                        pImageSliceMapper->SetCroppingRegion(v);
                    }
                }
                else if (const auto pImageResliceMapper = vtkImageResliceMapper::SafeDownCast(vtkObj); pImageResliceMapper && ImGui::CollapsingHeader("vtkImageResliceMapper", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ::vtkImageResliceMapper_setup(pImageResliceMapper);
                }

                if (const auto pActor = vtkActor::SafeDownCast(vtkObj); pActor && ImGui::CollapsingHeader("vtkActor", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkObjSetup("Property", pActor->GetProperty(), ImGuiTreeNodeFlags_DefaultOpen);
                }
                else if (const auto pImageSlice = vtkImageSlice::SafeDownCast(vtkObj); pImageSlice && ImGui::TreeNodeEx("vtkImageSlice", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkImageSlice_setup(pImageSlice);

                    if (const auto pImageActor = vtkImageActor::SafeDownCast(vtkObj); pImageActor && ImGui::TreeNodeEx("vtkImageActor", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        vtkImageActor_setup(pImageActor);

                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
                else if (const auto pVolume = vtkVolume::SafeDownCast(vtkObj); pVolume && ImGui::TreeNodeEx("vtkVolume", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    vtkObjSetup("Mapper", pVolume->GetMapper(), ImGuiTreeNodeFlags_DefaultOpen);
                    // property
                    if (const auto pVolumeProperty = pVolume->GetProperty(); pVolumeProperty && ImGui::TreeNodeEx("vtkVolumeProperty", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (bool v = pVolumeProperty->GetShade(); ImGui::Checkbox("ShadeOn", &v))
                        {
                            pVolumeProperty->SetShade(v);
                        }
                        ImGui::SameLine();
                        if (bool v = pVolumeProperty->GetDisableGradientOpacity(); ImGui::Checkbox("DisableGradientOpacity", &v))
                        {
                            pVolumeProperty->SetDisableGradientOpacity(v);
                        }
                        if (float v = pVolumeProperty->GetAmbient(); ImGui::SliderFloat("Ambient", &v, 0., 1.))
                        {
                            pVolumeProperty->SetAmbient(v);
                        }
                        if (float v = pVolumeProperty->GetDiffuse(); ImGui::SliderFloat("Diffuse", &v, 0., 1.))
                        {
                            pVolumeProperty->SetDiffuse(v);
                        }
                        if (float v = pVolumeProperty->GetSpecular(); ImGui::SliderFloat("Specular", &v, 0., 1.))
                        {
                            pVolumeProperty->SetSpecular(v);
                        }
                        if (float v = pVolumeProperty->GetSpecularPower(); ImGui::SliderFloat("SpecularPower", &v, 0., 300.))
                        {
                            pVolumeProperty->SetSpecularPower(v);
                        }
                        ImGui::Text("ColorChannels: %d", pVolumeProperty->GetColorChannels());

                        // InterpolationType
                        {
                            const char* modeText[] = { "VTK_NEAREST_INTERPOLATION", "VTK_LINEAR_INTERPOLATION", "VTK_CUBIC_INTERPOLATION" };
                            if (auto v = pVolumeProperty->GetInterpolationType(); ImGui::Combo("InterpolationType", &v, modeText, IM_ARRAYSIZE(modeText)))
                            {
                                pVolumeProperty->SetInterpolationType(v);
                            }
                        }
                        //ScalarOpacity
                        if (ImGui::CollapsingHeader("ScalarOpacity"))
                        {
                            const auto pPiecewiseFunction = pVolumeProperty->GetScalarOpacity();
                            if (ImGui::TreeNodeEx("vtkPiecewiseFunction", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                for (auto i = 0; i < pPiecewiseFunction->GetSize(); ++i)
                                {
                                    double nodeValue[4];
                                    pPiecewiseFunction->GetNodeValue(i, nodeValue);
                                    if (float v = nodeValue[1]; ImGui::SliderFloat(fmt::format("{}", nodeValue[0]).c_str(), &v, 0., 1.))
                                    {
                                        nodeValue[1] = v;
                                        pPiecewiseFunction->SetNodeValue(i, nodeValue);
                                    }
                                }

                                if (bool v = pPiecewiseFunction->GetClamping(); ImGui::Checkbox("Clamping", &v))
                                {
                                    pPiecewiseFunction->SetClamping(v);
                                }

                                ImGui::TreePop();
                            }
                        }
                        if (ImGui::CollapsingHeader("Color"))
                        {
                            const auto pColorTransferFunction = pVolumeProperty->GetRGBTransferFunction();
                            if (ImGui::TreeNodeEx("vtkColorTransferFunction", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                for (auto i = 0; i < pColorTransferFunction->GetSize(); ++i)
                                {
                                    double nodeValue[6];
                                    pColorTransferFunction->GetNodeValue(i, nodeValue);
                                    if (float v[3] = { nodeValue[1],nodeValue[2],nodeValue[3] }; ImGui::ColorEdit3(fmt::format("{}", nodeValue[0]).c_str(), v))
                                    {
#if 0 // 此代码无效
                                        nodeValue[1] = v[0];
                                        nodeValue[2] = v[1];
                                        nodeValue[3] = v[2];
                                        pColorTransferFunction->GetNodeValue(i, nodeValue);
                                        pColorTransferFunction->Modified();
#else
                                        pColorTransferFunction->AddRGBPoint(nodeValue[0], v[0], v[1], v[2]);
#endif
                                    }
                                }
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                else if (const auto pAnnotatedCubeActor = vtkAnnotatedCubeActor::SafeDownCast(vtkObj); pAnnotatedCubeActor && ImGui::TreeNodeEx("vtkAnnotatedCubeActor"))
                {
                    if (float v = vtkMath::RadiansFromDegrees(pAnnotatedCubeActor->GetXFaceTextRotation()); ImGui::SliderAngle("XFaceTextRotation", &v))
                    {
                        pAnnotatedCubeActor->SetXFaceTextRotation(vtkMath::DegreesFromRadians(v));
                    }
                    if (float v = vtkMath::RadiansFromDegrees(pAnnotatedCubeActor->GetYFaceTextRotation()); ImGui::SliderAngle("YFaceTextRotation", &v))
                    {
                        pAnnotatedCubeActor->SetYFaceTextRotation(vtkMath::DegreesFromRadians(v));
                    }
                    if (float v = vtkMath::RadiansFromDegrees(pAnnotatedCubeActor->GetZFaceTextRotation()); ImGui::SliderAngle("ZFaceTextRotation", &v))
                    {
                        pAnnotatedCubeActor->SetZFaceTextRotation(vtkMath::DegreesFromRadians(v));
                    }

                    vtkObjSetup("CubeProperty", pAnnotatedCubeActor->GetCubeProperty());
                    vtkObjSetup("TextEdgesProperty", pAnnotatedCubeActor->GetTextEdgesProperty());

                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }
}