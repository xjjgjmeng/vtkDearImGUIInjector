﻿#include "ImGuiCommon.h"

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <gl/GL.h>

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
    struct Data
    {
        GLuint out_texture;
        int out_width;
        int out_height;
    };
    static std::map<std::string, Data> mymap;

    try
    {
         const auto& data = mymap.at(file_name);
         *out_texture = data.out_texture;
         *out_width = data.out_width;
         *out_height = data.out_height;
         return true;
    }
    catch (const std::out_of_range& e)
    {
        FILE* f = std::fopen(file_name, "rb");
        if (f == NULL)
            return false;
        std::fseek(f, 0, SEEK_END);
        size_t file_size = (size_t)ftell(f);
        if (file_size == -1)
            return false;
        std::fseek(f, 0, SEEK_SET);
        void* file_data = IM_ALLOC(file_size);
        std::fread(file_data, 1, file_size, f);
        std::fclose(f);
        bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
        IM_FREE(file_data);

        mymap[file_name] = { *out_texture, *out_width, *out_height };

        return ret;
    }
}

namespace vtkns
{
    void image(const char* file_name)
    {
        int my_image_width = 0;
        int my_image_height = 0;
        GLuint my_image_texture = 0;
        bool ret = LoadTextureFromFile(file_name, &my_image_texture, &my_image_width, &my_image_height);
        IM_ASSERT(ret);
        //ImGui::Text("pointer = %x", my_image_texture);
        //ImGui::Text("size = %d x %d", my_image_width, my_image_height);
        ImGui::Image((ImTextureID)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
    }

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

    void HelpMarker(const char* desc, const char* file_name)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            vtkns::image(file_name);
            ImGui::EndTooltip();
        }
    }

    void HelpMarkerSameLine(const char* desc)
    {
        ImGui::SameLine();
        vtkns::HelpMarker(desc);
    }
}

namespace
{
    // 使用函数模板特化（且不提供默认实现），而不使用普通函数重载，可以避免在没有实现子类类型setupImpl的时候继续调用基类函数的问题
    template <typename T>
    void setupImpl(T*);

    template <>
    void setupImpl(vtkObject* vtkObj)
    {
        // ImGui::Text("Pointer: %0x", vtkObj);
        vtkns::ImGuiText(u8"地址: {}", (void*)vtkObj);
        ImGui::Text("MTime: %ld", vtkObj->GetMTime());
        ImGui::Text("Name: %s", vtkObj->GetClassName());
    }

    template <>
    void setupImpl(vtkActor2D* pActor2D)
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
        vtkns::vtkObjSetup("Property", pActor2D->GetProperty());
    }

    template<>
    void setupImpl(vtkTexturedActor2D* vtkobj)
    {
    
    }

    template<>
    void setupImpl(vtkAbstractContextItem* vtkobj)
    {
        vtkns::ImGuiText("NumberOfItems: {}", vtkobj->GetNumberOfItems());
        if (bool v = vtkobj->GetInteractive(); ImGui::Checkbox("Interactive ", &v))
        {
            vtkobj->SetInteractive(v);
        }
        if (bool v = vtkobj->GetVisible(); ImGui::Checkbox("Visible ", &v))
        {
            vtkobj->SetVisible(v);
        }
    }

    template<>
    void setupImpl(vtkContextItem* vtkobj)
    {
        if (float v = vtkobj->GetOpacity(); ImGui::DragFloat("Opacity", &v, 0.01, 0., 1.))
        {
            vtkobj->SetOpacity(v);
        }
    }

    template<>
    void setupImpl(vtkPlot* vtkobj)
    {
        {
            double vd[3];
            vtkobj->GetColor(vd);
            float v[3] = { vd[0], vd[1], vd[2] };
            if (ImGui::ColorEdit3("Color", v))
            {
                vtkobj->SetColor(v[0], v[1], v[2]);
            }
        }
        vtkns::ImGuiText("NumberOfLabels: {}", vtkobj->GetNumberOfLabels());
        if (bool v = vtkobj->GetLegendVisibility(); ImGui::Checkbox("LegendVisibility ", &v))
        {
            vtkobj->SetLegendVisibility(v);
        }
        if (bool v = vtkobj->GetSelectable(); ImGui::Checkbox("Selectable ", &v))
        {
            vtkobj->SetSelectable(v);
        }
        if (bool v = vtkobj->GetUseIndexForXSeries(); ImGui::Checkbox("UseIndexForXSeries ", &v))
        {
            vtkobj->SetUseIndexForXSeries(v);
        }
        if (float v = vtkobj->GetWidth(); ImGui::DragFloat("Width", &v, 0.01))
        {
            vtkobj->SetWidth(v);
        }
        vtkns::vtkObjSetup("XAxis", vtkobj->GetXAxis());
        vtkns::vtkObjSetup("YAxis", vtkobj->GetYAxis());
    }

    template<>
    void setupImpl(vtkControlPointsItem* vtkobj)
    {
        vtkns::ImGuiText("NumberOfPoints: {}", vtkobj->GetNumberOfPoints());
        {
            double v[4];
            vtkobj->GetBounds(v);
            ImGui::InputScalarN("Bounds", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        }
        if (double v[4]; vtkobj->GetUserBounds(v), ImGui::DragScalarN("UserBounds", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1))
        {
            vtkobj->SetUserBounds(v);
        }
        if (double v[4]; vtkobj->GetValidBounds(v), ImGui::DragScalarN("ValidBounds", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1))
        {
            vtkobj->SetValidBounds(v);
        }
        if (bool v = vtkobj->GetUseAddPointItem(); ImGui::Checkbox("UseAddPointItem ", &v))
        {
            vtkobj->SetUseAddPointItem(v);
        }
        if (bool v = vtkobj->GetShowLabels(); ImGui::Checkbox("ShowLabels ", &v))
        {
            vtkobj->SetShowLabels(v);
        }
        if (bool v = vtkobj->GetDrawPoints(); ImGui::Checkbox("DrawPoints ", &v))
        {
            vtkobj->SetDrawPoints(v);
        }
        if (bool v = vtkobj->GetStrokeMode(); ImGui::Checkbox("StrokeMode ", &v))
        {
            vtkobj->SetStrokeMode(v);
        }
        if (bool v = vtkobj->GetSwitchPointsMode(); ImGui::Checkbox("SwitchPointsMode ", &v))
        {
            vtkobj->SetSwitchPointsMode(v);
        }
        if (bool v = vtkobj->GetEndPointsXMovable(); ImGui::Checkbox("EndPointsXMovable ", &v))
        {
            vtkobj->SetEndPointsXMovable(v);
        }
        if (bool v = vtkobj->GetEndPointsYMovable(); ImGui::Checkbox("EndPointsYMovable ", &v))
        {
            vtkobj->SetEndPointsYMovable(v);
        }
        if (bool v = vtkobj->GetEndPointsMovable(); ImGui::Checkbox("EndPointsMovable ", &v))
        {
            //vtkobj->SetEndPointsMovable(v);
        }
        if (float v = vtkobj->GetScreenPointRadius(); ImGui::DragFloat("ScreenPointRadius", &v, 0.01))
        {
            vtkobj->SetScreenPointRadius(v);
        }
    }

    template<>
    void setupImpl(vtkColorTransferControlPointsItem* vtkobj)
    {
        if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx("ControlPoint", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (vtkIdType i = 0; i < vtkobj->GetNumberOfPoints(); ++i)
            {
                double v[4];
                vtkobj->GetControlPoint(i, v);
                vtkns::ImGuiText("{}: {}", i, v);
            }
        }
        if (bool v = vtkobj->GetColorFill(); ImGui::Checkbox("ColorFill ", &v))
        {
            vtkobj->SetColorFill(v);
        }
    }

    template<>
    void setupImpl(vtkCompositeControlPointsItem* vtkobj)
    {
        if (bool v = vtkobj->GetUseOpacityPointHandles(); ImGui::Checkbox("UseOpacityPointHandles ", &v))
        {
            vtkobj->SetUseOpacityPointHandles(v);
        }
        if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx("PointsFunction", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetPointsFunction();
            ImGui::RadioButton("ColorPointsFunction", &v, vtkCompositeControlPointsItem::ColorPointsFunction);
            ImGui::RadioButton("OpacityPointsFunction", &v, vtkCompositeControlPointsItem::OpacityPointsFunction);
            ImGui::RadioButton("ColorAndOpacityPointsFunction", &v, vtkCompositeControlPointsItem::ColorAndOpacityPointsFunction);
            if (vtkobj->GetPointsFunction() != v)
            {
                vtkobj->SetPointsFunction(v);
            }
        }
    }

    template<>
    void setupImpl(vtkPiecewiseControlPointsItem* vtkobj)
    {
    
    }

    template<>
    void setupImpl(vtkImageResize* vtkobj)
    {
        if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx("ResizeMethod", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetResizeMethod();
            ImGui::RadioButton("OUTPUT_DIMENSIONS", &v, vtkImageResize::OUTPUT_DIMENSIONS);
            ImGui::RadioButton("OUTPUT_SPACING", &v, vtkImageResize::OUTPUT_SPACING);
            ImGui::RadioButton("MAGNIFICATION_FACTORS", &v, vtkImageResize::MAGNIFICATION_FACTORS);
            if (vtkobj->GetResizeMethod() != v)
            {
                vtkobj->SetResizeMethod(v);
            }
        }

        if (int v[3]; vtkobj->GetOutputDimensions(v), ImGui::DragScalarN("OutputDimensions", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetOutputDimensions(v);
        }
        if (double v[3]; vtkobj->GetOutputSpacing(v), ImGui::DragScalarN("OutputSpacing", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1))
        {
            vtkobj->SetOutputSpacing(v);
        }
        if (double v[3]; vtkobj->GetMagnificationFactors(v), ImGui::DragScalarN("MagnificationFactors", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01))
        {
            vtkobj->SetMagnificationFactors(v);
        }
        if (bool v = vtkobj->GetBorder(); ImGui::Checkbox("Border ", &v))
        {
            vtkobj->SetBorder(v);
        }
        if (bool v = vtkobj->GetInterpolate(); ImGui::Checkbox("Interpolate ", &v))
        {
            vtkobj->SetInterpolate(v);
        }
        if (bool v = vtkobj->GetCropping(); ImGui::Checkbox("Cropping ", &v))
        {
            vtkobj->SetCropping(v);
        }
        if (double v[6]; vtkobj->GetCroppingRegion(v), ImGui::DragScalarN("CroppingRegion", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1))
        {
            vtkobj->SetCroppingRegion(v);
        }
        vtkns::vtkObjSetup("Interpolator", vtkobj->GetInterpolator());
    }

    template<>
    void setupImpl(vtkImageSincInterpolator* vtkobj)
    {
        if (bool v = vtkobj->GetAntialiasing(); ImGui::Checkbox("Antialiasing ", &v))
        {
            vtkobj->SetAntialiasing(v);
        }
        if (bool v = vtkobj->GetRenormalization(); ImGui::Checkbox("Renormalization ", &v))
        {
            vtkobj->SetRenormalization(v);
        }
        if (double v[3]; vtkobj->GetBlurFactors(v), ImGui::DragScalarN("BlurFactors", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1))
        {
            vtkobj->SetBlurFactors(v);
        }
        if (auto sg = nonstd::make_scope_exit(ImGui::TreePop); ImGui::TreeNodeEx("WindowFunction", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetWindowFunction();
            ImGui::RadioButton("VTK_LANCZOS_WINDOW", &v, VTK_LANCZOS_WINDOW);
            ImGui::RadioButton("VTK_KAISER_WINDOW", &v, VTK_KAISER_WINDOW);
            ImGui::RadioButton("VTK_COSINE_WINDOW", &v, VTK_COSINE_WINDOW);
            ImGui::RadioButton("VTK_HANN_WINDOW", &v, VTK_HANN_WINDOW);
            ImGui::RadioButton("VTK_HAMMING_WINDOW", &v, VTK_HAMMING_WINDOW);
            ImGui::RadioButton("VTK_BLACKMAN_WINDOW", &v, VTK_BLACKMAN_WINDOW);
            ImGui::RadioButton("VTK_BLACKMAN_HARRIS3", &v, VTK_BLACKMAN_HARRIS3);
            ImGui::RadioButton("VTK_BLACKMAN_HARRIS4", &v, VTK_BLACKMAN_HARRIS4);
            ImGui::RadioButton("VTK_NUTTALL_WINDOW", &v, VTK_NUTTALL_WINDOW);
            ImGui::RadioButton("VTK_BLACKMAN_NUTTALL3", &v, VTK_BLACKMAN_NUTTALL3);
            ImGui::RadioButton("VTK_BLACKMAN_NUTTALL4", &v, VTK_BLACKMAN_NUTTALL4);
            ImGui::RadioButton("VTK_SINC_KERNEL_SIZE_MAX", &v, VTK_SINC_KERNEL_SIZE_MAX);

            if (vtkobj->GetWindowFunction() != v)
            {
                vtkobj->SetWindowFunction(v);
            }
        }
    }

    template<>
    void setupImpl(vtkTextActor* vtkobj)
    {
        {
            double v[4];
            vtkobj->GetBoundingBox(::pRen, v);
            vtkns::ImGuiText("BoundingBox: {}", v);
        }
        {
            double v[2];
            vtkobj->GetSize(::pRen, v);
            vtkns::ImGuiText("Size: {}", v);
        }
        vtkns::vtkObjSetup("TextProperty", vtkobj->GetTextProperty());
    }

    template <>
    void setupImpl(vtkPointPlacer* vtkobj)
    {
        if (int v = vtkobj->GetPixelTolerance(); ImGui::DragInt("PixelTolerance", &v))
        {
            vtkobj->SetPixelTolerance(v);
        }
        if (int v = vtkobj->GetWorldTolerance(); ImGui::DragInt("WorldTolerance", &v))
        {
            vtkobj->SetWorldTolerance(v);
        }
    }

    template <>
    void setupImpl(vtkBoundedPlanePointPlacer* vtkobj)
    {
        {
            ImGui::Text("ProjectionNormal");
            ImGui::SameLine();
            int v = vtkobj->GetProjectionNormal();
            ImGui::RadioButton("XAxis", &v, vtkBoundedPlanePointPlacer::XAxis); ImGui::SameLine();
            ImGui::RadioButton("YAxis", &v, vtkBoundedPlanePointPlacer::YAxis); ImGui::SameLine();
            ImGui::RadioButton("ZAxis", &v, vtkBoundedPlanePointPlacer::ZAxis); ImGui::SameLine();
            ImGui::RadioButton("Oblique", &v, vtkBoundedPlanePointPlacer::Oblique);
            if (vtkobj->GetProjectionNormal() != v)
            {
                vtkobj->SetProjectionNormal(v);
            }
            if (vtkBoundedPlanePointPlacer::Oblique == v)
            {
                vtkns::vtkObjSetup("ObliquePlane", vtkobj->GetObliquePlane(), ImGuiTreeNodeFlags_DefaultOpen);
            }
        }

        if (float v = vtkobj->GetProjectionPosition(); ImGui::DragFloat("ProjectionPosition", &v))
        {
            vtkobj->SetProjectionPosition(v);
        }
    }

    template <>
    void setupImpl(vtkCutter* vtkobj)
    {
        if (bool v = vtkobj->GetGenerateTriangles(); ImGui::Checkbox("GenerateTriangles ", &v))
        {
            vtkobj->SetGenerateTriangles(v);
        }
        if (bool v = vtkobj->GetGenerateCutScalars(); ImGui::Checkbox("GenerateCutScalars ", &v))
        {
            vtkobj->SetGenerateCutScalars(v);
        }
        if (int v = vtkobj->GetNumberOfContours(); ImGui::DragInt("NumberOfContours", &v))
        {
            vtkobj->SetNumberOfContours(v);
        }
        if (int v = vtkobj->GetOutputPointsPrecision(); ImGui::DragInt("OutputPointsPrecision", &v))
        {
            vtkobj->SetOutputPointsPrecision(v);
        }
        {
            ImGui::Text("SortBy");
            ImGui::SameLine();
            int v = vtkobj->GetSortBy();
            ImGui::RadioButton("VALUE", &v, VTK_SORT_BY_VALUE); ImGui::SameLine();
            ImGui::RadioButton("CELL", &v, VTK_SORT_BY_CELL);
            if (vtkobj->GetSortBy() != v)
            {
                vtkobj->SetSortBy(v);
            }
        }
        vtkns::vtkObjSetup("CutFunction", vtkobj->GetCutFunction(), ImGuiTreeNodeFlags_DefaultOpen);
    }

    template<>
    void setupImpl(vtkImageChangeInformation* vtkobj)
    {
        if (double v[3]; vtkobj->GetOutputOrigin(v), ImGui::DragScalarN("OutputOrigin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetOutputOrigin(v);
        }
        if (double v[3]; vtkobj->GetOriginTranslation(v), ImGui::DragScalarN("OriginTranslation", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetOriginTranslation(v);
        }
        if (int v[3]; vtkobj->GetExtentTranslation(v), ImGui::DragScalarN("ExtentTranslation", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetExtentTranslation(v);
        }
        if (int v[3]; vtkobj->GetOutputExtentStart(v), ImGui::DragScalarN("OutputExtentStart", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetOutputExtentStart(v);
        }
        if (bool v = vtkobj->GetCenterImage(); ImGui::Checkbox("CenterImage", &v))
        {
            vtkobj->SetCenterImage(v);
        }
        ImGui::SameLine();
        vtkns::HelpMarker(u8R"(居中显示。通过Extent调整Origin，将整个Image的中心点和世界坐标的（0，0，0）对应)", "../../my_resource_dir/centerImage.png");
    }

    template<>
    void setupImpl(vtkScalarsToColors* vtkobj)
    {
        if (float v[2]{ vtkobj->GetRange()[0], vtkobj->GetRange()[1] }; ImGui::DragFloatRange2("Range", v, v + 1))
        {
            vtkobj->SetRange(v[0], v[1]);
        }
        if (double v = vtkobj->GetAlpha(); ImGui::DragScalar("Alpha", ImGuiDataType_Double, &v, 0.001f))
        {
            vtkobj->SetAlpha(v);
        }

        {
            ImGui::Text("VectorMode");
            ImGui::SameLine();
            int v = vtkobj->GetVectorMode();
            ImGui::RadioButton("COMPONENT", &v, vtkScalarsToColors::COMPONENT); ImGui::SameLine();
            ImGui::RadioButton("MAGNITUDE", &v, vtkScalarsToColors::MAGNITUDE); ImGui::SameLine();
            ImGui::RadioButton("RGBCOLORS", &v, vtkScalarsToColors::RGBCOLORS);
            if (vtkobj->GetVectorMode() != v)
            {
                vtkobj->SetVectorMode(v);
            }
        }

        if (ImGui::Button("Build"))
            vtkobj->Build();
    }

    template<>
    void setupImpl(vtkLookupTable* vtkobj)
    {
        if (float v[2]{ vtkobj->GetTableRange()[0], vtkobj->GetTableRange()[1] }; ImGui::DragFloatRange2("TableRange", v, v + 1))
        {
            vtkobj->SetTableRange(v[0], v[1]);
        }
        if (float v[2]{ vtkobj->GetHueRange()[0], vtkobj->GetHueRange()[1] }; ImGui::DragFloatRange2("HueRange", v, v + 1, 0.01, 0, 1))
        {
            vtkobj->SetHueRange(v[0], v[1]);
            vtkobj->Build();
        }
        if (float v[2]{ vtkobj->GetSaturationRange()[0], vtkobj->GetSaturationRange()[1] }; ImGui::DragFloatRange2("SaturationRange", v, v + 1, 0.01, 0, 1))
        {
            vtkobj->SetSaturationRange(v[0], v[1]);
            vtkobj->Build();
        }
        if (float v[2]{ vtkobj->GetValueRange()[0], vtkobj->GetValueRange()[1] }; ImGui::DragFloatRange2("ValueRange", v, v + 1, 0.01, 0, 1))
        {
            vtkobj->SetValueRange(v[0], v[1]);
            vtkobj->Build();
        }
        {
            ImGui::Text("Ramp");
            ImGui::SameLine();
            int v = vtkobj->GetRamp();
            ImGui::RadioButton("LINEAR", &v, VTK_RAMP_LINEAR); ImGui::SameLine();
            ImGui::RadioButton("SCURVE", &v, VTK_RAMP_SCURVE); ImGui::SameLine();
            ImGui::RadioButton("SQRT", &v, VTK_RAMP_SQRT);
            if (vtkobj->GetRamp() != v)
            {
                vtkobj->SetRamp(v);
            }
        }
        {
            static double currV = 0;
            currV = std::clamp(currV, vtkobj->GetTableRange()[0], vtkobj->GetTableRange()[1]);
            int v = vtkobj->GetIndex(currV);
            if (ImGui::DragScalar("GetIndex", ImGuiDataType_Double, &currV))
            {
                v = vtkobj->GetIndex(currV);
            }
            ImGui::SameLine();
            vtkns::ImGuiText("[{}]", v);
        }
        //vtkns::ImGuiText("NumberOfColors: {}", vtkobj->GetNumberOfColors());
        if (int v = vtkobj->GetNumberOfColors(); ImGui::DragInt("NumberOfColors", &v))
        {
            vtkobj->SetNumberOfColors(v);
            vtkobj->Build();
        }
        //vtkns::ImGuiText("NumberOfTableValues: {}", vtkobj->GetNumberOfTableValues());
        if (int v = vtkobj->GetNumberOfTableValues(); ImGui::DragInt("NumberOfTableValues", &v))
        {
            vtkobj->SetNumberOfTableValues(v);
            vtkobj->Build();
        }
        if (ImGui::TreeNodeEx("TableValues"))
        {
            for (auto i = 0; i < vtkobj->GetNumberOfTableValues(); ++i)
            {
                double vRGBA[4];
                vtkobj->GetTableValue(i, vRGBA);
                if (float v[4] = { vRGBA[0],vRGBA[1],vRGBA[2],vRGBA[3] }; ImGui::ColorEdit4(std::to_string(i).c_str(), v))
                {
                    vtkobj->SetTableValue(i, v[0], v[1], v[2], v[3]);
                }
            }

            ImGui::TreePop();
        }
    }

    template <>
    void setupImpl(vtkWidgetRepresentation* pWidgetRepresentation)
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

    template <>
    void setupImpl<vtkProp3D>(vtkProp3D* obj)
    {
        double pos[3];
        obj->GetPosition(pos);
        if (ImGui::DragScalarN("Position", ImGuiDataType_Double, pos, 3, 0.1f))
        {
            obj->SetPosition(pos);
        }

        double origin[3];
        obj->GetOrigin(origin);
        if (ImGui::DragScalarN("Origin", ImGuiDataType_Double, origin, 3, 0.1f))
        {
            obj->SetOrigin(origin);
        }

        double scale[3];
        obj->GetScale(scale);
        if (ImGui::DragScalarN("Scale", ImGuiDataType_Double, scale, 3, 0.001f))
        {
            obj->SetScale(scale);
        }

        ImGui::InputScalarN("Center", ImGuiDataType_Double, obj->GetCenter(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputScalarN("XRange", ImGuiDataType_Double, obj->GetXRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputScalarN("YRange", ImGuiDataType_Double, obj->GetYRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputScalarN("ZRange", ImGuiDataType_Double, obj->GetZRange(), 2, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputScalarN("Orientation", ImGuiDataType_Double, obj->GetOrientation(), 3, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
        ImGui::Text("IsIdentity: %s", obj->GetIsIdentity() ? "true" : "false");

        {
            float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            const auto n = 5;

            ImGui::Text("RotateX:");
            ImGui::SameLine();
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##leftRotateX", ImGuiDir_Left)) { obj->RotateX(-n); }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##rightRotateX", ImGuiDir_Right)) { obj->RotateX(n); }
            ImGui::PopButtonRepeat();

            ImGui::Text("RotateY:");
            ImGui::SameLine();
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##leftRotateY", ImGuiDir_Left)) { obj->RotateY(-n); }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##rightRotateY", ImGuiDir_Right)) { obj->RotateY(n); }
            ImGui::PopButtonRepeat();

            ImGui::Text("RotateZ:");
            ImGui::SameLine();
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##leftRotateZ", ImGuiDir_Left)) { obj->RotateZ(-n); }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##rightRotateZ", ImGuiDir_Right)) { obj->RotateZ(n); }
            ImGui::PopButtonRepeat();
        }
    }

    template <>
    void setupImpl(vtkAbstractMapper* pAbstractMapper)
    {
        ImGui::Text(fmt::format("NumberOfClippingPlanes: {}", pAbstractMapper->GetNumberOfClippingPlanes()).c_str());
        if (ImGui::TreeNodeEx("ClippingPlanes"))
        {
            auto pc = pAbstractMapper->GetClippingPlanes();
            for (auto i = 0; i < pc->GetNumberOfItems(); ++i)
            {
                vtkns::vtkObjSetup(("Plane" + std::to_string(i)).c_str(), pc->GetItem(i));
            }

            ImGui::TreePop();
        }
    }

    template <>
    void setupImpl(vtkMapper* obj)
    {

    }

    template <>
    void setupImpl(vtkPolyDataMapper* obj)
    {
        ImGui::InputScalarN("Bounds", ImGuiDataType_Double, obj->GetBounds(), 6, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
    }

    template <>
    void setupImpl(vtkImageGridSource* obj)
    {
        if (double v = obj->GetLineValue(); ImGui::DragScalar("LineValue", ImGuiDataType_Double, &v)) obj->SetLineValue(v);
        if (double v = obj->GetFillValue(); ImGui::DragScalar("FillValue", ImGuiDataType_Double, &v)) obj->SetFillValue(v);
        if (int v[3]; obj->GetGridSpacing(v), ImGui::DragScalarN("GridSpacing", ImGuiDataType_S32, v, IM_ARRAYSIZE(v))) obj->SetGridSpacing(v);
        if (int v[3]; obj->GetGridOrigin(v), ImGui::DragScalarN("GridOrigin", ImGuiDataType_S32, v, IM_ARRAYSIZE(v))) obj->SetGridOrigin(v);
        if (int v[6]; obj->GetDataExtent(v), ImGui::DragScalarN("DataExtent", ImGuiDataType_S32, v, IM_ARRAYSIZE(v))) obj->SetDataExtent(v);
        if (double v[3]; obj->GetDataOrigin(v), ImGui::DragScalarN("DataOrigin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v))) obj->SetDataOrigin(v);
        if (double v[3]; obj->GetDataSpacing(v), ImGui::DragScalarN("DataSpacing", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01f)) obj->SetDataSpacing(v);
    }

    template <>
    void setupImpl(vtkElevationFilter* obj)
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

    template <>
    void setupImpl(vtkImageAlgorithm* vtkobj)
    {
        vtkns::vtkObjSetup("Output", vtkobj->GetOutput());
    }

    template <>
    void setupImpl(vtkResliceCursorWidget* obj)
    {}

    template <>
    void setupImpl(vtkLineWidget2* obj)
    {}

    template <>
    void setupImpl(vtkAxisActor2D* pAxisActor2D)
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
        vtkns::vtkObjSetup("TitleTextProperty", pAxisActor2D->GetTitleTextProperty());
    }

    template <>
    void setupImpl(vtkTextProperty* pTextProperty)
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
        if (bool v = pTextProperty->GetUseTightBoundingBox(); ImGui::Checkbox("UseTightBoundingBox ", &v))
        {
            pTextProperty->SetUseTightBoundingBox(v);
        }
        if (int v = pTextProperty->GetInteriorLinesWidth(); ImGui::DragInt("InteriorLinesWidth", &v))
        {
            pTextProperty->SetInteriorLinesWidth(v);
        }
        if (float v[3] = { pTextProperty->GetInteriorLinesColor()[0],pTextProperty->GetInteriorLinesColor()[1],pTextProperty->GetInteriorLinesColor()[2] }; ImGui::ColorEdit3("InteriorLinesColor", v))
        {
            pTextProperty->SetInteriorLinesColor(v[0], v[1], v[2]);
        }
        if (double v = pTextProperty->GetLineOffset(); ImGui::DragScalar("LineOffset", ImGuiDataType_Double, &v, 0.01f))
        {
            pTextProperty->SetLineOffset(v);
        }
        if (double v = pTextProperty->GetLineSpacing(); ImGui::DragScalar("LineSpacing", ImGuiDataType_Double, &v, 0.01f))
        {
            pTextProperty->SetLineSpacing(v);
        }
        if (double v = pTextProperty->GetCellOffset(); ImGui::DragScalar("CellOffset", ImGuiDataType_Double, &v, 0.01f))
        {
            pTextProperty->SetCellOffset(v);
        }
        if (double v = pTextProperty->GetOrientation(); ImGui::DragScalar("Orientation", ImGuiDataType_Double, &v, 0.01f))
        {
            pTextProperty->SetOrientation(v);
        }
        if (bool v = pTextProperty->GetShadow(); ImGui::Checkbox("Shadow", &v))
        {
            pTextProperty->SetShadow(v);
        }
        if (int v[2]; pTextProperty->GetShadowOffset(v), ImGui::DragInt2("ShadowOffset", v))
        {
            pTextProperty->SetShadowOffset(v);
        }
        if (float v = pTextProperty->GetFontSize(); ImGui::SliderFloat("FontSize", &v, 0, 100))
        {
            pTextProperty->SetFontSize(v);
        }
        if (ImGui::Button("SetFontFamilyToArial")) pTextProperty->SetFontFamilyToArial();
        if (ImGui::Button("SetFontFamilyToCourier")) pTextProperty->SetFontFamilyToCourier();
        if (ImGui::Button("SetFontFamilyToTimes")) pTextProperty->SetFontFamilyToTimes();

        {
            ImGui::Text("Justification");
            ImGui::SameLine();
            int v = pTextProperty->GetJustification();
            ImGui::RadioButton("LEFT", &v, VTK_TEXT_LEFT); ImGui::SameLine();
            ImGui::RadioButton("H_CENTERED", &v, VTK_TEXT_CENTERED); ImGui::SameLine();
            ImGui::RadioButton("RIGHT", &v, VTK_TEXT_RIGHT);
            if (pTextProperty->GetJustification() != v)
            {
                pTextProperty->SetJustification(v);
            }
        }
        {
            ImGui::Text("VerticalJustification");
            ImGui::SameLine();
            int v = pTextProperty->GetVerticalJustification();
            ImGui::RadioButton("BOTTOM", &v, VTK_TEXT_BOTTOM); ImGui::SameLine();
            ImGui::RadioButton("V_CENTERED", &v, VTK_TEXT_CENTERED); ImGui::SameLine();
            ImGui::RadioButton("TOP", &v, VTK_TEXT_TOP);
            if (pTextProperty->GetVerticalJustification() != v)
            {
                pTextProperty->SetVerticalJustification(v);
            }
        }
    }

    template <>
    void setupImpl(vtkProperty2D* pProperty2D)
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

    template <>
    void setupImpl(vtkVolumeMapper* pVolumeMapper)
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

        vtkns::vtkObjSetup("Input", pVolumeMapper->GetInput());
    }

    template <>
    void setupImpl(vtkGPUVolumeRayCastMapper* pGPUVolumeRayCastMapper)
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
    }

    template <>
    void setupImpl(vtkThreadedImageAlgorithm*)
    {}

    template <>
    void setupImpl(vtkImageMapToColors* vtkobj)
    {
        vtkns::vtkObjSetup("LookupTable", vtkobj->GetLookupTable());
        if (bool v = vtkobj->GetPassAlphaToOutput(); ImGui::Checkbox("PassAlphaToOutput ", &v))
        {
            vtkobj->SetPassAlphaToOutput(v);
        }
    }

    template <>
    void setupImpl(vtkDataSetAlgorithm* obj)
    {

    }

    template <>
    void setupImpl(vtkPolyDataSourceWidget* obj)
    {
    
    }

    template <>
    void setupImpl(vtkPlaneWidget* obj)
    {
        {
            vtkNew<vtkPlane> plane;// ?
            obj->GetPlane(plane.GetPointer());
            vtkns::vtkObjSetup("Plane(ReadOnly)", plane, ImGuiTreeNodeFlags_DefaultOpen);
        }
        {
            vtkNew<vtkPolyData> polyData;
            obj->GetPolyData(polyData.GetPointer());
            vtkns::vtkObjSetup("PolyData(ReadOnly)", polyData);
        }
        if (int v = obj->GetResolution(); ImGui::DragInt("Resolution", &v))
        {
            obj->SetResolution(v);
        }
        if (double v[3]; obj->GetOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetOrigin(v);
        }
        if (double v[3]; obj->GetPoint1(v), ImGui::DragScalarN("Point1", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetPoint1(v);
        }
        if (double v[3]; obj->GetPoint2(v), ImGui::DragScalarN("Point2", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetPoint2(v);
        }
        if (double v[3]; obj->GetNormal(v), ImGui::DragScalarN("Normal", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetNormal(v);
        }
        if (double v[3]; obj->GetCenter(v), ImGui::DragScalarN("Center", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetCenter(v);
        }
        ImGui::SameLine(); vtkns::HelpMarker(u8R"(影响vtkPlane的origin)");
        if (ImGui::TreeNodeEx("Representation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = obj->GetRepresentation();
            ImGui::RadioButton("VTK_PLANE_OFF", &v, VTK_PLANE_OFF);
            ImGui::RadioButton("VTK_PLANE_OUTLINE", &v, VTK_PLANE_OUTLINE);
            ImGui::RadioButton("VTK_PLANE_WIREFRAME", &v, VTK_PLANE_WIREFRAME);
            ImGui::RadioButton("VTK_PLANE_SURFACE", &v, VTK_PLANE_SURFACE);
            if (obj->GetRepresentation() != v)
            {
                obj->SetRepresentation(v); // 不自动刷新
            }
            ImGui::TreePop();
        }
        if (bool v = obj->GetNormalToXAxis(); ImGui::Checkbox("NormalToXAxis ", &v))
        {
            obj->SetNormalToXAxis(v); // 无效
        }
        if (bool v = obj->GetNormalToYAxis(); ImGui::Checkbox("NormalToYAxis ", &v))
        {
            obj->SetNormalToYAxis(v);
        }
        if (bool v = obj->GetNormalToZAxis(); ImGui::Checkbox("NormalToZAxis ", &v))
        {
            obj->SetNormalToZAxis(v);
        }
        vtkns::vtkObjSetup("HandleProperty", obj->GetHandleProperty());
        vtkns::vtkObjSetup("SelectedHandleProperty", obj->GetSelectedHandleProperty());
        vtkns::vtkObjSetup("PlaneProperty", obj->GetPlaneProperty());
        vtkns::vtkObjSetup("SelectedPlaneProperty", obj->GetSelectedPlaneProperty());
    }

    template <>
    void setupImpl(vtkImageToPolyDataFilter* obj)
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

    template <>
    void setupImpl(vtkPlaneSource* obj)
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
        if (double v[3]; obj->GetOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetOrigin(v);
        }
        if (double v[3]; obj->GetPoint1(v), ImGui::DragScalarN("Point1", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetPoint1(v);
        }
        if (double v[3]; obj->GetPoint2(v), ImGui::DragScalarN("Point2", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetPoint2(v);
        }
        if (double v[3]; obj->GetCenter(v), ImGui::DragScalarN("Center", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetCenter(v);
        }
        ImGui::SameLine();
        vtkns::HelpMarker(u8R"(移动整个plane，plane的size不变，Point1和Point2和Origin会被修改)");
        if (double v[3]; obj->GetNormal(v), ImGui::DragScalarN("Normal", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
        {
            obj->SetNormal(v);
        }
    }

    template <>
    void setupImpl(vtkMatrix4x4* obj)
    {
        if (ImGui::TreeNodeEx("Data", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::DragScalarN("Row0", ImGuiDataType_Double, obj->GetData()+ 0, 4, 0.1f)) obj->Modified();
            if (ImGui::DragScalarN("Row1", ImGuiDataType_Double, obj->GetData()+ 4, 4, 0.1f)) obj->Modified();
            if (ImGui::DragScalarN("Row2", ImGuiDataType_Double, obj->GetData()+ 8, 4, 0.1f)) obj->Modified();
            if (ImGui::DragScalarN("Row3", ImGuiDataType_Double, obj->GetData()+12, 4, 0.1f)) obj->Modified();
            ImGui::TreePop();
        }
        ImGui::Text(fmt::format("Determinant:{}", obj->Determinant()).c_str());
        if (ImGui::Button("Invert")) obj->Invert(); ImGui::SameLine();
        if (ImGui::Button("Transpose")) obj->Transpose(); ImGui::SameLine();
        if (ImGui::Button("Zero")) obj->Zero(); ImGui::SameLine();
        if (ImGui::Button("Identity")) obj->Identity(); ImGui::SameLine();
        obj->IsIdentity() ? ImGui::Text("IsIdentity") : ImGui::TextDisabled("IsIdentity");
        ImGui::Text(vtkns::getMatrixString(obj).c_str());
        {
            if (ImGui::TreeNodeEx(u8"操作", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static bool useVtkTransform = true;
                ImGui::Checkbox("UseVtkTransform", &useVtkTransform);
                ImGui::SameLine();
                vtkns::HelpMarker(u8R"(使用vtkTransform(选中)，使用自定义矩阵(非选中))");
                if (auto b = ImGui::TreeNodeEx(u8"平移", ImGuiTreeNodeFlags_DefaultOpen); ImGui::SameLine(), vtkns::HelpMarker(u8R"(每次沿轴移动1)"), b)
                {
                    vtkns::ArrowButton("X", [&] { vtkns::mat::translate(obj, -1, 0, 0, useVtkTransform); }, [&] { vtkns::mat::translate(obj, 1, 0, 0, useVtkTransform); }); vtkns::ArrowButtonSameLine();
                    vtkns::ArrowButton("Y", [&] { vtkns::mat::translate(obj, 0, -1, 0, useVtkTransform); }, [&] { vtkns::mat::translate(obj, 0, 1, 0, useVtkTransform); }); vtkns::ArrowButtonSameLine();
                    vtkns::ArrowButton("Z", [&] { vtkns::mat::translate(obj, 0, 0, -1, useVtkTransform); }, [&] { vtkns::mat::translate(obj, 0, 0, 1, useVtkTransform); });
                    ImGui::TreePop();
                }
                if (auto b = ImGui::TreeNodeEx(u8"旋转", ImGuiTreeNodeFlags_DefaultOpen); ImGui::SameLine(), vtkns::HelpMarker(u8R"(每次绕轴旋转5°)"), b)
                {
                    vtkns::ArrowButton("X", [&] { vtkns::mat::rotate(obj, 0, -1, useVtkTransform); }, [&] { vtkns::mat::rotate(obj, 0, 1, useVtkTransform); }); vtkns::ArrowButtonSameLine();
                    vtkns::ArrowButton("Y", [&] { vtkns::mat::rotate(obj, 1, -1, useVtkTransform); }, [&] { vtkns::mat::rotate(obj, 1, 1, useVtkTransform); }); vtkns::ArrowButtonSameLine();
                    vtkns::ArrowButton("Z", [&] { vtkns::mat::rotate(obj, 2, -1, useVtkTransform); }, [&] { vtkns::mat::rotate(obj, 2, 1, useVtkTransform); });
                    ImGui::TreePop();
                }
                if (auto b = ImGui::TreeNodeEx(u8"缩放", ImGuiTreeNodeFlags_DefaultOpen); ImGui::SameLine(), vtkns::HelpMarker(u8R"(每次缩放0.5或2)"), b)
                {
                    constexpr auto n = 0.5;
                    constexpr auto p = 2;
                    vtkns::ArrowButton("X", [&] { vtkns::mat::scale(obj, n, 1, 1, useVtkTransform); }, [&] { vtkns::mat::scale(obj, p, 1, 1, useVtkTransform); }); vtkns::ArrowButtonSameLine();
                    vtkns::ArrowButton("Y", [&] { vtkns::mat::scale(obj, 1, n, 1, useVtkTransform); }, [&] { vtkns::mat::scale(obj, 1, p, 1, useVtkTransform); }); vtkns::ArrowButtonSameLine();
                    vtkns::ArrowButton("Z", [&] { vtkns::mat::scale(obj, 1, 1, n, useVtkTransform); }, [&] { vtkns::mat::scale(obj, 1, 1, p, useVtkTransform); });
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
    }

    template <>
    void setupImpl(vtkImageSlab* pImageSlab)
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
    }

    template <>
    void setupImpl(vtkImageThreshold* pImageThreshold)
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
            pImageThreshold->Update(); // 不update输出的imagedata是二维
        }
        if (float v = pImageThreshold->GetOutValue(); ImGui::DragFloat("OutValue", &v))
        {
            pImageThreshold->SetOutValue(v);
            pImageThreshold->Update(); // 不update输出的imagedata是二维
        }
        {
            float l = pImageThreshold->GetLowerThreshold();
            float u = pImageThreshold->GetUpperThreshold();
            if (ImGui::DragFloatRange2("ThresholdBetween", &l, &u))
            {
                pImageThreshold->ThresholdBetween(l, u);
                pImageThreshold->Update(); // 不update输出的imagedata是二维
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

    template <>
    void setupImpl(vtkAlgorithm* pAlgorithm)
    {
        if (ImGui::Button("Update"))
        {
            pAlgorithm->Update();
        }

        if (ImGui::Button("UpdateInformation"))
        {
            pAlgorithm->UpdateInformation();
        }
    }

    template <>
    void setupImpl(vtkAbstractImageInterpolator* obj)
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

    template <>
    void setupImpl(vtkImageReslice* obj)
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
        vtkns::vtkObjSetup(u8"矩阵", obj->GetResliceAxes(), ImGuiTreeNodeFlags_DefaultOpen);
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
                    //obj->Update();
                }
                if (ImGui::DragScalarN("DirectionCosinesY", ImGuiDataType_Double, xyz + 3, 3, .01f))
                {
                    obj->SetResliceAxesDirectionCosines(xyz);
                    //obj->Update();
                }
                if (ImGui::DragScalarN("DirectionCosinesZ", ImGuiDataType_Double, xyz + 6, 3, .01f))
                {
                    obj->SetResliceAxesDirectionCosines(xyz);
                    //obj->Update();
                }

                if (double v[3]; obj->GetResliceAxesOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .1f))
                {
                    obj->SetResliceAxesOrigin(v);
                    //obj->Update();
                } ImGui::SameLine(); vtkns::HelpMarker(u8R"(指定reslice坐标系的origin在旧世界中的位置
调节该值相当于reslice坐标系在旧世界中移动
xoy应该可以穿过imagedata，不然无输出
-固定output的origin和extent不变，修改ResliceAxes的origin的x和y，发现黑窗在世界中不动，image在黑窗口中游走
-固定ResliceAxes的origin，修改output的origin，发现黑窗在世界中游走，可以看到image不同部分
-固定ResliceAxes的origin，修改output的extent，发现黑窗在世界扩大或缩小，可以看到image的更多或更少
output的origin是相对于新坐标系的，把新坐标系的origin处看作（0，0，0），但最后在原始的坐标系中也是一样的)");
                if (ImGui::Button(u8"重置XYZ"))
                {
                    obj->SetResliceAxesDirectionCosines(1, 0, 0, 0, 1, 0, 0, 0, 1);
                }

                ImGui::TreePop();
            }
        }

        // output
        if (auto b = ImGui::TreeNodeEx("Output", ImGuiTreeNodeFlags_DefaultOpen); ImGui::SameLine(), vtkns::HelpMarker(u8R"(调节Origin和Extent，效果就是在钉在世界0点的xoy图像上选取合适的图像区域，
用Origin在xoy图像上确定输出图像的Origin，此点也就是输出图像的extent (0,0,0)，
在此基础上使用Extent选择一个区域，即为输出图像)"), b)
        {
            if (double v[3]; obj->GetOutputOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
            {
                obj->SetOutputOrigin(v);
                //obj->Update(); // 没有此句会输出的都是二维. 使用SetInputConnection就不需要？？
            } ImGui::SameLine(); vtkns::HelpMarker(u8R"(reslice坐标系的origin永远处于新世界的（0，0，0）处
（想象：将reslice的矩阵放置在旧世界中，此矩阵的xoy面会切出一帧图像，矩阵已经和整体的imagedata完成绑定。此刻拖拽着矩阵会带着imagedata，将矩阵的xyz轴和origin和旧世界进行对齐，就得到了imagedata的新世界坐标）
OutpuOrigin是相对于新世界的
The origin is the position in world coordinates of the point of extent (0,0,0)
如果输出的是2维，调节z无效，因为只reslice出一张图，所以z被忽略了？？输出图像只能在一个平面上游移)");
            //::reslice->GetOutputInformation(0)->Get(vtkDataObject::SPACING(), myArray); // 0.25
            if (double v[3]; obj->GetOutputSpacing(v), ImGui::DragScalarN("Spacing", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), .01f))
            {
                obj->SetOutputSpacing(v);
                //obj->Update(); // 没有此句会输出的都是二维
            } ImGui::SameLine(); vtkns::HelpMarker(u8R"(和vtkImageData的spacing不同。
通过调大spacing再配合网格可以看到虽然输出的图像尺寸变大了，但是对应像素的位置是不变的，通过采样？？
确定了spacing就相当于确定了网格，后面的extent只是选取想要的网格索引来进一步reslice)");
            if (int v[6]; obj->GetOutputExtent(v), ImGui::DragScalarN("Extent", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
            {
                obj->SetOutputExtent(v);
                //obj->Update(); // 没有此句会输出的都是二维
            } ImGui::SameLine(); vtkns::HelpMarker(u8R"(新的slice已经切割出来，此属性控制可以看到的图像范围
1. 通过reslice在xoy平面切出图像
2. 通过OutputOrigin指定输出图像的extent (0,0,0)的位置
3. 通过OutputExtent选取输出区域)");

            if (ImGui::Button("SetOutputOriginToDefault"))
            {
                obj->SetOutputOriginToDefault();
                //obj->Update();
            }
            ImGui::SameLine();
            if (ImGui::Button("SetOutputExtentToDefault"))
            {
                obj->SetOutputExtentToDefault();
                //obj->Update();
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
                    //obj->Update(); // 没有此句会输出的都是二维
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
        vtkns::vtkObjSetup("Stencil", obj->GetStencil());
    }

    template <>
    void setupImpl(vtkMarchingCubes* obj)
    {
        if (double v = obj->GetValue(0); ImGui::InputDouble("Value", &v, 500.f, 500.0f, "%.8f"))
        {
            obj->SetValue(0, v);
        }
        if (bool v = obj->GetComputeNormals(); ImGui::Checkbox("ComputeNormals", &v))
        {
            obj->SetComputeNormals(v);
        }
        if (bool v = obj->GetComputeScalars(); ImGui::Checkbox("ComputeScalars", &v))
        {
            obj->SetComputeScalars(v);
        }
        if (bool v = obj->GetComputeGradients(); ImGui::Checkbox("ComputeGradients", &v))
        {
            obj->SetComputeGradients(v);
        }
        if (int v = obj->GetNumberOfContours(); ImGui::DragInt("NumberOfContours", &v))
        {
            obj->SetNumberOfContours(v);
        }
    }

    template <>
    void setupImpl(vtkFlyingEdges3D* obj)
    {
        if (double v = obj->GetValue(0); ImGui::InputDouble("Value", &v, 500.f, 500.0f, "%.8f"))
        {
            obj->SetValue(0, v);
        }
        if (bool v = obj->GetComputeNormals(); ImGui::Checkbox("ComputeNormals", &v))
        {
            obj->SetComputeNormals(v);
        }
        if (bool v = obj->GetComputeScalars(); ImGui::Checkbox("ComputeScalars", &v))
        {
            obj->SetComputeScalars(v);
        }
        if (bool v = obj->GetComputeGradients(); ImGui::Checkbox("ComputeGradients", &v))
        {
            obj->SetComputeGradients(v);
        }
        if (int v = obj->GetNumberOfContours(); ImGui::DragInt("NumberOfContours", &v))
        {
            obj->SetNumberOfContours(v);
        }
    }

    template <>
    void setupImpl(vtkScalarBarActor* obj)
    {
        if (int v = obj->GetNumberOfLabels(); ImGui::SliderInt("NumberOfLabels", &v, 0, 100))
        {
            obj->SetNumberOfLabels(v);
        }
        if (int v = obj->GetVerticalTitleSeparation(); ImGui::SliderInt("VerticalTitleSeparation", &v, 0, 100))
        {
            obj->SetVerticalTitleSeparation(v);
        }
        if (ImGui::TreeNodeEx("TextPosition", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = obj->GetTextPosition();
            ImGui::RadioButton("PrecedeScalarBar", &v, vtkScalarBarActor::PrecedeScalarBar);
            ImGui::RadioButton("SucceedScalarBar", &v, vtkScalarBarActor::SucceedScalarBar);
            if (obj->GetTextPosition() != v)
            {
                obj->SetTextPosition(v);
            }
            ImGui::TreePop();
        }
        if (bool v = obj->GetDrawAnnotations(); ImGui::Checkbox("DrawAnnotations ", &v))
        {
            obj->SetDrawAnnotations(v);
        }
    }

    template <>
    void setupImpl(vtkImageViewer2* pImageViewer2)
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
    }

    template <>
    void setupImpl(vtkDistanceRepresentation* pDistanceRepresentation)
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
        vtkns::vtkObjSetup("Point1Representation", pDistanceRepresentation->GetPoint1Representation());
        vtkns::vtkObjSetup("Point2Representation", pDistanceRepresentation->GetPoint2Representation());
    }

    template <>
    void setupImpl(vtkLineRepresentation* pLineRepresentation)
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
        vtkns::vtkObjSetup("EndPointProperty", pLineRepresentation->GetEndPointProperty());
        vtkns::vtkObjSetup("EndPoint2Property", pLineRepresentation->GetEndPoint2Property());
        vtkns::vtkObjSetup("LineProperty", pLineRepresentation->GetLineProperty());
        vtkns::vtkObjSetup("SelectedLineProperty", pLineRepresentation->GetSelectedLineProperty());
        vtkns::vtkObjSetup("DistanceAnnotationProperty", pLineRepresentation->GetDistanceAnnotationProperty());
    }

    template <>
    void setupImpl(vtkBoxRepresentation* pBoxRepresentation)
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

    template <>
    void setupImpl(vtkBorderRepresentation* pBorderRepresentation)
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

    template <>
    void setupImpl(vtkHandleRepresentation* pHandleRepresentation)
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

    template <>
    void setupImpl(vtkTransformPolyDataFilter* obj)
    {
    }

    template <>
    void setupImpl(vtkDistanceWidget* pDistanceWidget)
    {
        {
            const char* modeText[] = { "Start", "Define", "Manipulate" };
            ImGui::Text(fmt::format("WidgetState: {}", modeText[pDistanceWidget->GetWidgetState()]).c_str());
        }
    }

    template <>
    void setupImpl(vtkBoxWidget2* pBoxWidget2)
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
    }

    template <>
    void setupImpl(vtkBorderWidget* pBorderWidget)
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

    template <>
    void setupImpl(vtkResliceCursor* pResliceCursor)
    {
        if (bool v = pResliceCursor->GetThickMode(); ImGui::Checkbox("ThickMode ", &v))
        {
            pResliceCursor->SetThickMode(v);
        }

        if (double v[3]; pResliceCursor->GetThickness(v), ImGui::DragScalarN("Thickness", ImGuiDataType_Double, v, 3, 0.5f))
        {
            pResliceCursor->SetThickness(v);
        }
    }

    template <>
    void setupImpl(vtkAbstractWidget* pAbstractWidget)
    {
        if (bool v = pAbstractWidget->GetManagesCursor(); ImGui::Checkbox("ManagesCursor ", &v))
        {
            pAbstractWidget->SetManagesCursor(v);
        }
        if (bool v = pAbstractWidget->GetProcessEvents(); ImGui::Checkbox("ProcessEvents ", &v))
        {
            pAbstractWidget->SetProcessEvents(v);
        }
        vtkns::vtkObjSetup("Representation", pAbstractWidget->GetRepresentation(), ImGuiTreeNodeFlags_DefaultOpen);
    }

    template <>
    void setupImpl(vtkInteractorObserver* pInteractorObserver)
    {
        if (bool v = pInteractorObserver->GetEnabled(); ImGui::Checkbox("Enabled ", &v))
        {
            pInteractorObserver->SetEnabled(v);
        }

        if (ImGui::Button("On")) pInteractorObserver->On(); ImGui::SameLine();
        if (ImGui::Button("Off")) pInteractorObserver->Off();
    }

    template <>
    void setupImpl(vtkDataObject* obj)
    {
        ImGui::Text(fmt::format("ActualMemorySize: {}", obj->GetActualMemorySize()).c_str());
        ImGui::Text(fmt::format("DataObjectType: {}", obj->GetDataObjectType()).c_str());
        ImGui::Text(fmt::format("ExtentType: {}", obj->GetExtentType()).c_str());
    }

    template <>
    void setupImpl(vtkImageSliceMapper* pImageSliceMapper)
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

    template <>
    void setupImpl(vtkDataSet* obj)
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

    template <>
    void setupImpl(vtkPointSet* obj)
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

    template <>
    void setupImpl(vtkImageData* obj)
    {
        vtkns::ImGuiText("NumberOfScalarComponents: {}", obj->GetNumberOfScalarComponents());
        vtkns::ImGuiText("DataDimension: {}", obj->GetDataDimension());
        vtkns::ImGuiText("ScalarType: {}", obj->GetScalarTypeAsString());
        vtkns::ImGuiText("ScalarSize: {}", obj->GetScalarSize());
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

    template <>
    void setupImpl(vtkImageResliceMapper* obj)
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

    template <>
    void setupImpl(vtkResliceImageViewer* pResliceImageViewer)
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

        vtkns::vtkObjSetup("ResliceCursor", pResliceImageViewer->GetResliceCursor(), ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("ResliceCursorWidget", pResliceImageViewer->GetResliceCursorWidget(), ImGuiTreeNodeFlags_DefaultOpen);
    }

    template <>
    void setupImpl(vtkImageMapper3D* obj)
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

    template <>
    void setupImpl(vtkImplicitFunction* obj)
    {

    }

    template <>
    void setupImpl(vtkAbstractMapper3D* pAbstractMapper3D)
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

    template <>
    void setupImpl(vtkPlane* obj)
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

        if (double v[3]; obj->GetOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.05f))
        {
            obj->SetOrigin(v);
        }

        if (double v[3]; obj->GetNormal(v), ImGui::DragScalarN("Normal", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.01f))
        {
            obj->SetNormal(v);
        }
    }

    template <>
    void setupImpl(vtkVolumeOutlineSource* pVolumeOutlineSource)
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
    }

    template <>
    void setupImpl(vtkImageSlice* obj)
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

    template <>
    void setupImpl(vtkImageProperty* obj)
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

    template <>
    void setupImpl(vtkAnnotatedCubeActor* pAnnotatedCubeActor)
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

        vtkns::vtkObjSetup("CubeProperty", pAnnotatedCubeActor->GetCubeProperty());
        vtkns::vtkObjSetup("TextEdgesProperty", pAnnotatedCubeActor->GetTextEdgesProperty());
    }

    template <>
    void setupImpl(vtkExtractVOI* pExtractVOI)
    {
        if (int v[6]; pExtractVOI->GetVOI(v), ImGui::DragScalarN("VOI", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
        {
            pExtractVOI->SetVOI(v);
            pExtractVOI->Update();
        }

        {
            int minV = 1;
            int maxV = 8;
            if (int v[3]; pExtractVOI->GetSampleRate(v), ImGui::DragScalarN("SampleRate", ImGuiDataType_S32, v, IM_ARRAYSIZE(v), 1.f, &minV, &maxV))
            {
                pExtractVOI->SetSampleRate(v);
                pExtractVOI->Update();
            }
        }
    }

    template <>
    void setupImpl(vtkImageActor* obj)
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

    template <>
    void setupImpl(vtkImageFlip* obj)
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

    template <>
    void setupImpl(vtkPolyDataAlgorithm* pPolyDataAlgorithm)
    {
        vtkns::vtkObjSetup("Output", pPolyDataAlgorithm->GetOutput());
    }

    template<>
    void setupImpl(vtkGlyph3D* vtkobj)
    {
        if (bool v = vtkobj->GetScaling(); ImGui::Checkbox("Scaling", &v))
        {
            vtkobj->SetScaling(v);
        }
        if (ImGui::TreeNodeEx("ScaleMode", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetScaleMode();
            ImGui::RadioButton("VTK_SCALE_BY_SCALAR", &v, VTK_SCALE_BY_SCALAR);
            ImGui::RadioButton("VTK_SCALE_BY_VECTOR", &v, VTK_SCALE_BY_VECTOR);
            ImGui::RadioButton("VTK_SCALE_BY_VECTORCOMPONENTS", &v, VTK_SCALE_BY_VECTORCOMPONENTS);
            ImGui::RadioButton("VTK_DATA_SCALING_OFF", &v, VTK_DATA_SCALING_OFF);
            if (vtkobj->GetScaleMode() != v)
            {
                vtkobj->SetScaleMode(v);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("ColorMode", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetColorMode();
            ImGui::RadioButton("VTK_SCALE_BY_SCALAR", &v, VTK_COLOR_BY_SCALE);
            ImGui::RadioButton("VTK_SCALE_BY_VECTOR", &v, VTK_COLOR_BY_SCALAR);
            ImGui::RadioButton("VTK_SCALE_BY_VECTORCOMPONENTS", &v, VTK_COLOR_BY_VECTOR);
            if (vtkobj->GetColorMode() != v)
            {
                vtkobj->SetColorMode(v);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("VectorMode", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetVectorMode();
            ImGui::RadioButton("VTK_USE_VECTOR", &v, VTK_USE_VECTOR);
            ImGui::RadioButton("VTK_USE_NORMAL", &v, VTK_USE_NORMAL);
            ImGui::RadioButton("VTK_VECTOR_ROTATION_OFF", &v, VTK_VECTOR_ROTATION_OFF);
            ImGui::RadioButton("VTK_FOLLOW_CAMERA_DIRECTION", &v, VTK_FOLLOW_CAMERA_DIRECTION);
            if (vtkobj->GetVectorMode() != v)
            {
                vtkobj->SetVectorMode(v);
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("IndexMode", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetIndexMode();
            ImGui::RadioButton("VTK_INDEXING_OFF", &v, VTK_INDEXING_OFF);
            ImGui::RadioButton("VTK_INDEXING_BY_SCALAR", &v, VTK_INDEXING_BY_SCALAR);
            ImGui::RadioButton("VTK_INDEXING_BY_VECTOR", &v, VTK_INDEXING_BY_VECTOR);
            if (vtkobj->GetIndexMode() != v)
            {
                vtkobj->SetIndexMode(v);
            }
            ImGui::TreePop();
        }
        if (double v = vtkobj->GetScaleFactor(); ImGui::DragScalar("ScaleFactor", ImGuiDataType_Double, &v, 0.1f))
        {
            vtkobj->SetScaleFactor(v);
        }
        if (double v[2]; vtkobj->GetRange(v), ImGui::DragScalarN("Range", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
        {
            vtkobj->SetRange(v);
        }
        if (bool v = vtkobj->GetOrient(); ImGui::Checkbox("Orient", &v))
        {
            vtkobj->SetOrient(v);
        }
        if (bool v = vtkobj->GetGeneratePointIds(); ImGui::Checkbox("GeneratePointIds", &v))
        {
            vtkobj->SetGeneratePointIds(v);
        }
        vtkns::ImGuiText("PointIdsName: {}", vtkobj->GetPointIdsName());
        if (bool v = vtkobj->GetClamping(); ImGui::Checkbox("Clamping", &v))
        {
            vtkobj->SetClamping(v);
        }
        if (bool v = vtkobj->GetFillCellData(); ImGui::Checkbox("FillCellData", &v))
        {
            vtkobj->SetFillCellData(v);
        }
        if (double v[3]; vtkobj->GetFollowedCameraPosition(v), ImGui::DragScalarN("FollowedCameraPosition", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
        {
            vtkobj->SetFollowedCameraPosition(v);
        }
        if (double v[3]; vtkobj->GetFollowedCameraViewUp(v), ImGui::DragScalarN("FollowedCameraViewUp", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
        {
            vtkobj->SetFollowedCameraViewUp(v);
        }
        if (int v = vtkobj->GetOutputPointsPrecision(); ImGui::DragInt("OutputPointsPrecision", &v))
        {
            vtkobj->SetOutputPointsPrecision(v);
        }
    }

    template<>
    void setupImpl(vtkGlyph2D* vtkobj)
    {

    }

    template<>
    void setupImpl(vtkGlyphSource2D* vtkobj)
    {
        if (double v = vtkobj->GetRotationAngle(); ImGui::DragScalar("RotationAngle", ImGuiDataType_Double, &v, 0.1f))
        {
            vtkobj->SetRotationAngle(v);
        }
        if (double v = vtkobj->GetScale(); ImGui::DragScalar("Scale", ImGuiDataType_Double, &v, 0.1f))
        {
            vtkobj->SetScale(v);
        }
        if (double v = vtkobj->GetScale2(); ImGui::DragScalar("Scale2", ImGuiDataType_Double, &v, 0.1f))
        {
            vtkobj->SetScale2(v);
        }
        if (double v[3]; vtkobj->GetCenter(v), ImGui::DragScalarN("Center", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.1f))
        {
            vtkobj->SetCenter(v);
        }
        if (int v = vtkobj->GetResolution(); ImGui::DragInt("Resolution", &v))
        {
            vtkobj->SetResolution(v);
        }
        if (int v = vtkobj->GetOutputPointsPrecision(); ImGui::DragInt("OutputPointsPrecision", &v))
        {
            vtkobj->SetOutputPointsPrecision(v);
        }

        {
            float color[3] = { vtkobj->GetColor()[0],vtkobj->GetColor()[1],vtkobj->GetColor()[2] };
            if (ImGui::ColorEdit3("Color", color))
            {
                vtkobj->SetColor(color[0], color[1], color[2]);
            }
        }

        if (bool v = vtkobj->GetFilled(); ImGui::Checkbox("Filled", &v))
        {
            vtkobj->SetFilled(v);
        }

        if (bool v = vtkobj->GetDash(); ImGui::Checkbox("Dash", &v))
        {
            vtkobj->SetDash(v);
        }

        if (bool v = vtkobj->GetCross(); ImGui::Checkbox("Cross", &v))
        {
            vtkobj->SetCross(v);
        }

        if (ImGui::TreeNodeEx("GlyphType", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int v = vtkobj->GetGlyphType();
            ImGui::RadioButton("VTK_NO_GLYPH", &v, VTK_NO_GLYPH);
            ImGui::RadioButton("VTK_VERTEX_GLYPH", &v, VTK_VERTEX_GLYPH);
            ImGui::RadioButton("VTK_DASH_GLYPH", &v, VTK_DASH_GLYPH);
            ImGui::RadioButton("VTK_CROSS_GLYPH", &v, VTK_CROSS_GLYPH);
            ImGui::RadioButton("VTK_THICKCROSS_GLYPH", &v, VTK_THICKCROSS_GLYPH);
            ImGui::RadioButton("VTK_TRIANGLE_GLYPH", &v, VTK_TRIANGLE_GLYPH);
            ImGui::RadioButton("VTK_SQUARE_GLYPH", &v, VTK_SQUARE_GLYPH);
            ImGui::RadioButton("VTK_CIRCLE_GLYPH", &v, VTK_CIRCLE_GLYPH);
            ImGui::RadioButton("VTK_DIAMOND_GLYPH", &v, VTK_DIAMOND_GLYPH);
            ImGui::RadioButton("VTK_ARROW_GLYPH", &v, VTK_ARROW_GLYPH);
            ImGui::RadioButton("VTK_THICKARROW_GLYPH", &v, VTK_THICKARROW_GLYPH);
            ImGui::RadioButton("VTK_HOOKEDARROW_GLYPH", &v, VTK_HOOKEDARROW_GLYPH);
            ImGui::RadioButton("VTK_EDGEARROW_GLYPH", &v, VTK_EDGEARROW_GLYPH);
            if (vtkobj->GetGlyphType() != v)
            {
                vtkobj->SetGlyphType(v);
            }
            ImGui::TreePop();
        }
    }

    template <>
    void setupImpl(vtk3DWidget* obj)
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

    template <>
    void setupImpl(vtkLineSource* pLineSource)
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
    }

    template <>
    void setupImpl(vtkPointWidget* obj)
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

    template <>
    void setupImpl(vtkWindow* obj)
    {
        if (bool v = obj->GetErase(); ImGui::Checkbox("Erase", &v))
        {
            obj->SetErase(v);
        }
    }

    template <>
    void setupImpl(vtkRenderWindow* obj)
    {
        ImGui::Text(fmt::format("NumberOfLayers: {}", obj->GetNumberOfLayers()).c_str());
        if (int v = obj->GetMultiSamples(); ImGui::SliderInt("MultiSamples", &v, 0, 10)) obj->SetMultiSamples(v);
    }

    template <>
    void setupImpl(vtkActor* pActor)
    {
        vtkns::vtkObjSetup("Property", pActor->GetProperty(), ImGuiTreeNodeFlags_DefaultOpen);
    }

    template <>
    void setupImpl(vtkCaptionActor2D* obj)
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
        vtkns::vtkObjSetup("TextActor", obj->GetTextActor());
        vtkns::vtkObjSetup("CaptionTextProperty", obj->GetCaptionTextProperty());
    }

    template <>
    void setupImpl(vtkCamera* obj)
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

    template <>
    void setupImpl(vtkVolumeProperty* pVolumeProperty)
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
    }

    template <>
    void setupImpl(vtkAbstractPropPicker* pAbstractPropPicker)
    {
        vtkns::vtkObjSetup("ViewProp", pAbstractPropPicker->GetViewProp());
        ImGui::Text(fmt::format("ViewProp: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetViewProp())).c_str());
        ImGui::Text(fmt::format("Prop3D: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetProp3D())).c_str());
        ImGui::Text(fmt::format("Actor2D: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetActor2D())).c_str());
        ImGui::Text(fmt::format("Actor: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetActor())).c_str());
        ImGui::Text(fmt::format("Volume: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetVolume())).c_str());
        ImGui::Text(fmt::format("Assembly: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetAssembly())).c_str());
        ImGui::Text(fmt::format("PropAssembly: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetPropAssembly())).c_str());
        ImGui::Text(fmt::format("Path: {}", reinterpret_cast<void*>(pAbstractPropPicker->GetPath())).c_str());
    }

    template <>
    void setupImpl(vtkAbstractPicker* pAbstractPicker)
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
    }

    template <>
    void setupImpl(vtkPicker* pPicker)
    {
        if (double v = pPicker->GetTolerance(); ImGui::DragScalar("Tolerance", ImGuiDataType_Double, &v, 0.01f))
        {
            pPicker->SetTolerance(v);
        }
        ImGui::Text(fmt::format("Actors-NumberOfItems: {}", pPicker->GetActors()->GetNumberOfItems()).c_str());
        ImGui::Text(fmt::format("Prop3Ds-NumberOfItems: {}", pPicker->GetProp3Ds()->GetNumberOfItems()).c_str());
    }

    template <>
    void setupImpl(vtkPointPicker* pPointPicker)
    {
        ImGui::Text(fmt::format("PointId: {}", pPointPicker->GetPointId()).c_str());
    }

    template <>
    void setupImpl(vtkCellPicker* pCellPicker)
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
    }

    template <>
    void setupImpl(vtkPropPicker* obj)
    {}

    template <>
    void setupImpl(vtkWorldPointPicker* obj)
    {}

    template <>
    void setupImpl(vtkProperty* obj)
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

    template <>
    void setupImpl(vtkImageStencilData* vtkobj)
    {
        if (double v[3]; vtkobj->GetOrigin(v), ImGui::DragScalarN("Origin", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetOrigin(v);
        }
        if (int v[6]; vtkobj->GetExtent(v), ImGui::DragScalarN("Extent", ImGuiDataType_S32, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetExtent(v);
        }
        if (double v[3]; vtkobj->GetSpacing(v), ImGui::DragScalarN("Spacing", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            vtkobj->SetSpacing(v);
        }
    }

    template <>
    void setupImpl(vtkImageStencilAlgorithm* vtkobj)
    {
        vtkns::vtkObjSetup("Output", vtkobj->GetOutput());
    }

    template <>
    void setupImpl(vtkImageToImageStencil* vtkobj)
    {
        {
            float l = vtkobj->GetLowerThreshold();
            float u = vtkobj->GetUpperThreshold();
            if (ImGui::DragFloatRange2("ThresholdBetween", &l, &u))
            {
                vtkobj->ThresholdBetween(l, u);
            }
            if (ImGui::DragFloat("ThresholdByLower", &u))
            {
                vtkobj->ThresholdByLower(u);
            }
            if (ImGui::DragFloat("ThresholdByUpper", &l))
            {
                vtkobj->ThresholdByUpper(l);
            }
        }
    }

    template <>
    void setupImpl(vtkImageStencil* vtkobj)
    {
        if (bool v = vtkobj->GetReverseStencil(); ImGui::Checkbox("ReverseStencil", &v))
        {
            vtkobj->SetReverseStencil(v);
        }

        if (double v = vtkobj->GetBackgroundValue(); ImGui::DragScalar("BackgroundValue", ImGuiDataType_Double, &v, 0.01f))
        {
            vtkobj->SetBackgroundValue(v);
        }

        {
            auto bgColor = vtkobj->GetBackgroundColor();
            if (float v[4] = { bgColor[0],bgColor[1],bgColor[2],bgColor[3]}; ImGui::ColorEdit4("BackgroundColor", v))
            {
                vtkobj->SetBackgroundColor(v[0], v[1], v[2], v[3]);
            }
        }

        vtkns::vtkObjSetup("Stencil", vtkobj->GetStencil());
        vtkns::vtkObjSetup("BackgroundInput", vtkobj->GetBackgroundInput());
        ImGui::SameLine();
        vtkns::HelpMarker(R"(void vtkImageStencil::SetBackgroundInputData(vtkImageData* data)
{
  this->SetInputData(1, data);
}
)");
    }

    template <>
    void setupImpl(vtkPointHandleRepresentation2D* pPointHandleRepresentation2D)
    {
        if (ImGui::Button("Highlight")) pPointHandleRepresentation2D->Highlight(1);
    }

    template <>
    void setupImpl(vtkDistanceRepresentation2D* pDistanceRepresentation2D)
    {
        vtkns::vtkObjSetup("Axis", pDistanceRepresentation2D->GetAxis());
        vtkns::vtkObjSetup("AxisProperty", pDistanceRepresentation2D->GetAxisProperty());
    }

    template <>
    void setupImpl(vtkViewport* pViewport)
    {
        if (float v[3] = { pViewport->GetBackground()[0],pViewport->GetBackground()[1],pViewport->GetBackground()[2] }; ImGui::ColorEdit3("Background", v))
        {
            pViewport->SetBackground(v[0], v[1], v[2]);
        }
    }

    template <>
    void setupImpl(vtkRenderer* pRenderer)
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
        if (bool v = pRenderer->GetUseDepthPeeling(); ImGui::Checkbox("UseDepthPeeling", &v))
        {
            pRenderer->SetUseDepthPeeling(v);
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

    template <>
    void setupImpl(vtkProp* pProp)
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

    template <>
    void setupImpl(vtkVolume* pVolume)
    {
        vtkns::vtkObjSetup("Mapper", pVolume->GetMapper(), ImGuiTreeNodeFlags_DefaultOpen);
        vtkns::vtkObjSetup("Property", pVolume->GetProperty());
    }

    template <>
    void setupImpl(vtkAxesActor* obj)
    {
        {
            double bounds[6];
            obj->GetBounds(bounds);
            vtkns::ImGuiText("Bounds: {::.2f}", bounds);
        }

        vtkns::ImGuiText("XAxisLabelText: {}", obj->GetXAxisLabelText());
        vtkns::ImGuiText("YAxisLabelText: {}", obj->GetYAxisLabelText());
        vtkns::ImGuiText("ZAxisLabelText: {}", obj->GetZAxisLabelText());

        if (double v[3]; obj->GetTotalLength(v), ImGui::DragScalarN("TotalLength", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetTotalLength(v);
        }
        if (double v[3]; obj->GetNormalizedShaftLength(v), ImGui::DragScalarN("NormalizedShaftLength", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetNormalizedShaftLength(v);
        }
        if (double v[3]; obj->GetNormalizedTipLength(v), ImGui::DragScalarN("NormalizedTipLength", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetNormalizedTipLength(v);
        }
        if (double v[3]; obj->GetNormalizedLabelPosition(v), ImGui::DragScalarN("NormalizedLabelPosition", ImGuiDataType_Double, v, IM_ARRAYSIZE(v)))
        {
            obj->SetNormalizedLabelPosition(v);
        }

        if (int v = obj->GetConeResolution(); ImGui::SliderInt("ConeResolution", &v, 0, 100))
        {
            obj->SetConeResolution(v);
        }
        if (int v = obj->GetSphereResolution(); ImGui::SliderInt("SphereResolution", &v, 0, 100))
        {
            obj->SetSphereResolution(v);
        }
        if (int v = obj->GetCylinderResolution(); ImGui::SliderInt("CylinderResolution", &v, 0, 100))
        {
            obj->SetCylinderResolution(v);
        }

        if (double v = obj->GetConeRadius(); ImGui::DragScalar("ConeRadius", ImGuiDataType_Double, &v, 0.01f))
        {
            obj->SetConeRadius(v);
        }
        if (double v = obj->GetSphereRadius(); ImGui::DragScalar("SphereRadius", ImGuiDataType_Double, &v, 0.01f))
        {
            obj->SetSphereRadius(v);
        }
        if (double v = obj->GetCylinderRadius(); ImGui::DragScalar("CylinderRadius", ImGuiDataType_Double, &v, 0.01f))
        {
            obj->SetCylinderRadius(v);
        }

        {
            ImGui::Text("ShaftType");
            ImGui::SameLine();
            int v = obj->GetShaftType();
            ImGui::RadioButton("CYLINDER_SHAFT", &v, vtkAxesActor::CYLINDER_SHAFT); ImGui::SameLine();
            ImGui::RadioButton("LINE_SHAFT", &v, vtkAxesActor::LINE_SHAFT); ImGui::SameLine();
            ImGui::RadioButton("USER_DEFINED_SHAFT", &v, vtkAxesActor::USER_DEFINED_SHAFT);
            if (obj->GetShaftType() != v)
            {
                obj->SetShaftType(v);
            }
        }

        {
            ImGui::Text("TipType");
            ImGui::SameLine();
            int v = obj->GetTipType();
            ImGui::RadioButton("CONE_TIP", &v, vtkAxesActor::CONE_TIP); ImGui::SameLine();
            ImGui::RadioButton("SPHERE_TIP", &v, vtkAxesActor::SPHERE_TIP); ImGui::SameLine();
            ImGui::RadioButton("USER_DEFINED_TIP", &v, vtkAxesActor::USER_DEFINED_TIP);
            if (obj->GetTipType() != v)
            {
                obj->SetTipType(v);
            }
        }

        if (bool v = obj->GetAxisLabels(); ImGui::Checkbox("AxisLabels", &v))
        {
            obj->SetAxisLabels(v);
        }

        vtkns::vtkObjSetup("XAxisCaptionActor2D", obj->GetXAxisCaptionActor2D());
        vtkns::vtkObjSetup("YAxisCaptionActor2D", obj->GetYAxisCaptionActor2D());
        vtkns::vtkObjSetup("ZAxisCaptionActor2D", obj->GetZAxisCaptionActor2D());
    }

    template <>
    void setupImpl(vtkOrientationMarkerWidget* obj)
    {
        {
            float color[3] = { obj->GetOutlineColor()[0],obj->GetOutlineColor()[1],obj->GetOutlineColor()[2] };
            if (ImGui::ColorEdit3("OutlineColor", color))
            {
                obj->SetOutlineColor(color[0], color[1], color[2]);
            }
        }

        if (bool v = obj->GetInteractive(); ImGui::Checkbox("Interactive", &v))
        {
            obj->SetInteractive(v);
        }
        if (bool v = obj->GetShouldConstrainSize(); ImGui::Checkbox("ShouldConstrainSize", &v))
        {
            obj->SetShouldConstrainSize(v);
        }

        if (double v[4]; obj->GetViewport(v), ImGui::DragScalarN("Viewport", ImGuiDataType_Double, v, IM_ARRAYSIZE(v), 0.001f))
        {
            obj->SetViewport(v);
        }

        if (double v = obj->GetZoom(); ImGui::DragScalar("Zoom", ImGuiDataType_Double, &v, 0.01f))
        {
            obj->SetZoom(v);
        }

        if (int v = obj->GetTolerance(); ImGui::SliderInt("Tolerance", &v, 1, 10))
        {
            obj->SetTolerance(v);
        }
    }
}

namespace
{
    template <typename T, typename... Ts>
    void setupHelper(vtkObject* obj)
    {
        constexpr auto flags = std::is_same_v<T, vtkObject> ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_DefaultOpen;
        if (const auto p = T::SafeDownCast(obj); p && ImGui::CollapsingHeader(typeid(T).name(), flags))
        {
            ::setupImpl<T>(p);
        }
        else
        {
            if constexpr (sizeof...(Ts))
            {
                ::setupHelper<Ts...>(obj);
            }
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

        if (objName.empty() || ImGui::TreeNodeEx(objName.data(), flags)) // ! name为空直接通过，否则才TreeNode判断
        {
            ::setupHelper<vtkObject>(vtkObj);

            // vtkObject
            ::setupHelper<vtkProp
                , vtkCamera
                , vtkViewport
                , vtkDataObject
                , vtkAbstractPicker
                , vtkProperty
                , vtkImageViewer2
                , vtkResliceCursor
                , vtkInteractorObserver
                , vtkAlgorithm
                , vtkTextProperty
                , vtkProperty2D
                , vtkImplicitFunction
                , vtkImageProperty
                , vtkWindow
                , vtkAbstractImageInterpolator
                , vtkMatrix4x4
                , vtkPointPlacer
                , vtkVolumeProperty
                , vtkAbstractContextItem
                , vtkScalarsToColors>(vtkObj);
            // vtkAbstractContextItem
            ::setupHelper<vtkContextItem>(vtkObj);
            // vtkContextItem
            ::setupHelper<vtkPlot>(vtkObj);
            // vtkPlot
            ::setupHelper<vtkControlPointsItem>(vtkObj);
            // vtkControlPointsItem
            ::setupHelper<vtkColorTransferControlPointsItem, vtkPiecewiseControlPointsItem>(vtkObj);
            // vtkColorTransferControlPointsItem
            ::setupHelper<vtkCompositeControlPointsItem>(vtkObj);
            // vtkPointPlacer
            ::setupHelper<vtkBoundedPlanePointPlacer>(vtkObj);
            // vtkAbstractImageInterpolator
            ::setupHelper<vtkImageSincInterpolator>(vtkObj);
            // vtkViewport
            ::setupHelper<vtkRenderer>(vtkObj);
            // vtkScalarsToColors
            ::setupHelper<vtkLookupTable>(vtkObj);
            // vtkDataObject
            ::setupHelper<vtkDataSet, vtkImageStencilData>(vtkObj);
            // vtkDataSet
            ::setupHelper<vtkImageData, vtkPointSet>(vtkObj);
            // vtkAbstractPicker
            ::setupHelper<vtkAbstractPropPicker, vtkWorldPointPicker>(vtkObj);
            // vtkAbstractPropPicker
            ::setupHelper<vtkPicker, vtkPropPicker>(vtkObj);
            // vtkPicker
            ::setupHelper<vtkPointPicker, vtkCellPicker>(vtkObj);
            // vtkInteractorObserver
            ::setupHelper<vtkAbstractWidget, vtk3DWidget, vtkOrientationMarkerWidget>(vtkObj);
            // vtkAbstractWidget
            ::setupHelper<vtkResliceCursorWidget, vtkBoxWidget2, vtkLineWidget2, vtkDistanceWidget, vtkBorderWidget>(vtkObj);
            // vtkAlgorithm
            ::setupHelper<vtkImageAlgorithm, vtkImageStencilAlgorithm, vtkPolyDataAlgorithm, vtkAbstractMapper, vtkDataSetAlgorithm>(vtkObj);
            // vtkImageStencilAlgorithm
            ::setupHelper<vtkImageToImageStencil>(vtkObj);
            // vtkImageAlgorithm
            ::setupHelper<vtkThreadedImageAlgorithm, vtkExtractVOI, vtkImageGridSource, vtkImageChangeInformation>(vtkObj);
            // vtkAbstractMapper
            ::setupHelper<vtkAbstractMapper3D>(vtkObj);
            // vtkThreadedImageAlgorithm
            ::setupHelper<vtkImageResize, vtkImageSlab, vtkImageStencil, vtkImageThreshold, vtkImageReslice, vtkImageMapToColors>(vtkObj);
            // vtkImageViewer2
            ::setupHelper<vtkResliceImageViewer>(vtkObj);
            // vtkWindow
            ::setupHelper<vtkRenderWindow>(vtkObj);
            // vtkDataSetAlgorithm
            ::setupHelper<vtkElevationFilter>(vtkObj);
            // vtkPolyDataAlgorithm
            ::setupHelper<vtkLineSource, vtkCutter, vtkGlyphSource2D, vtkGlyph3D, vtkVolumeOutlineSource, vtkFlyingEdges3D, vtkMarchingCubes, vtkTransformPolyDataFilter, vtkPlaneSource, vtkImageToPolyDataFilter>(vtkObj);
            // vtkGlyph3D
            ::setupHelper<vtkGlyph2D>(vtkObj);
            // vtkAbstractMapper3D
            ::setupHelper<vtkMapper, vtkImageMapper3D>(vtkObj);
            // vtkMapper
            ::setupHelper<vtkPolyDataMapper>(vtkObj);
            // vtkImplicitFunction
            ::setupHelper<vtkPlane>(vtkObj);
            // vtk3DWidget
            ::setupHelper<vtkPointWidget, vtkPolyDataSourceWidget>(vtkObj);
            // vtkPolyDataSourceWidget
            ::setupHelper<vtkPlaneWidget>(vtkObj);
            // vtkProp
            ::setupHelper<vtkProp3D, vtkActor2D, vtkWidgetRepresentation>(vtkObj);
            // vtkProp3D
            ::setupHelper<vtkImageSlice, vtkActor, vtkVolume, vtkAnnotatedCubeActor, vtkAxesActor>(vtkObj);
            // vtkImageSlice
            ::setupHelper<vtkImageActor>(vtkObj);
            // vtkAbstractVolumeMapper
            ::setupHelper<vtkVolumeMapper>(vtkObj);
            // vtkVolumeMapper
            ::setupHelper<vtkGPUVolumeRayCastMapper>(vtkObj);
            // vtkActor2D
            ::setupHelper<vtkAxisActor2D, vtkCaptionActor2D, vtkTexturedActor2D, vtkScalarBarActor>(vtkObj);
            // vtkTexturedActor2D
            ::setupHelper<vtkTextActor>(vtkObj);
            // vtkWidgetRepresentation
            ::setupHelper<vtkBoxRepresentation, vtkLineRepresentation, vtkDistanceRepresentation, vtkBorderRepresentation, vtkHandleRepresentation>(vtkObj);
            // vtkHandleRepresentation
            ::setupHelper<vtkPointHandleRepresentation2D>(vtkObj);
            // vtkDistanceRepresentation
            ::setupHelper<vtkDistanceRepresentation2D>(vtkObj);
            // vtkImageReslice
            ::setupHelper<vtkImageFlip>(vtkObj);
            // vtkImageMapper3D
            ::setupHelper<vtkImageSliceMapper, vtkImageResliceMapper>(vtkObj);

            if (!objName.empty())
            {
                ImGui::TreePop();
            }
        }
    }
}