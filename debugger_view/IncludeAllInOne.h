#pragma once

#include <Windows.h>

#include <sstream>
#include <string>
#include <string_view>
#include <functional>
#include <filesystem>

#include "vtkDearImGuiInjector.h"

#include "vtkActor.h"
#include "vtkCallbackCommand.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkCameraOrientationRepresentation.h"
#include "vtkCameraOrientationWidget.h"
#include "vtkConeSource.h"
#include "vtkCubeSource.h"
#include "vtkInteractorStyle.h"
#include "vtkInteractorStyleSwitch.h"
#include "vtkNew.h"
#include "vtkImageReslice.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include <vtkPolyDataMapper2D.h>
#include "vtkRenderer.h"
#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkRenderWindow.h"
#include "vtkImageMapToColors.h"
#include "vtkLookupTable.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkDICOMImageReader.h"
#include "vtkImageActor.h"
#include "vtkInteractorStyleImage.h"
#include <vtkNamedColors.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkResliceImageViewer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkResliceImageViewerMeasurements.h>
#include <vtkResliceCursorLineRepresentation.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceCursorActor.h>
#include <vtkResliceCursorPolyDataAlgorithm.h>
#include <vtkImageViewer2.h>
#include <vtkCellPicker.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkPropPicker.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkSphereSource.h>
#include <vtkCamera.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkPlaneSource.h>
#include <vtkPropAssembly.h>
#include <vtkInformation.h>
#include <vtkMath.h>
#include <vtkTexture.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkLineSource.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageActorPointPlacer.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include "vtkfmt/core.h"
#include "vtkfmt/ranges.h"
#include "vtkfmt/format.h"
#include "vtkfmt/chrono.h"

#include <imgui.h>

static std::function<void()> imgui_render_callback;

namespace ImguiVtkNs
{
    static void DrawUI(vtkDearImGuiInjector* overlay)
    {
        vtkNew<vtkCallbackCommand> uiDraw;
        auto uiDrawFunction = [](vtkObject* caller, long unsigned int vtkNotUsed(eventId),
            void* clientData, void* vtkNotUsed(callData))
            {
                vtkDearImGuiInjector* overlay_ = reinterpret_cast<vtkDearImGuiInjector*>(caller);

                ImGui::SetNextWindowPos(ImVec2(0, 25), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(450, 550), ImGuiCond_Once);
                ImGui::Begin("VTK");
                {
                    if (::imgui_render_callback)
                    {
                        ::imgui_render_callback();
                    }
                }
                ImGui::End();
            };
        uiDraw->SetCallback(uiDrawFunction);
        overlay->AddObserver(vtkDearImGuiInjector::ImGuiDrawEvent, uiDraw);
    }

    static void SetupUI(vtkDearImGuiInjector* overlay)
    {
        vtkNew<vtkCallbackCommand> uiSetup;
        auto uiSetupFunction =
            [](vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* callData)
            {
                vtkDearImGuiInjector* overlay_ = reinterpret_cast<vtkDearImGuiInjector*>(caller);
                if (!callData)
                {
                    return;
                }
                bool imguiInitStatus = *(reinterpret_cast<bool*>(callData));
                if (imguiInitStatus)
                {
                    auto io = ImGui::GetIO();
                    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf", 15.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
                }
            };
        uiSetup->SetCallback(uiSetupFunction);
        overlay->AddObserver(vtkDearImGuiInjector::ImGuiSetupEvent, uiSetup);
    }

    static const char* getDicomFile()
    {
        const char* retval = "D:/test_data/202110020082000/255.dcm";
        if (!std::filesystem::exists(retval))
        {
            throw "dicom file does not exist!";
        }
        return retval;
    }

    static const char* getDicomDir()
    {
        const char* retval = "D:/test_data/202110020082000";
        if (!std::filesystem::exists(retval))
        {
            throw "dicom dir does not exist!";
        }
        return retval;
    }
}