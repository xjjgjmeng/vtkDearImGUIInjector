#pragma once

#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkPointPicker.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCaptionWidget.h>
#include <vtkCellPicker.h>
#include <vtkPropPicker.h>
#include <vtkProp3D.h>
#include <vtkSelectionNode.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractSelection.h>
#include <vtkActor2D.h>
#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkfmt/core.h>
#include <vtkfmt/format.h>
#include "vtkfmt/core.h"
#include "vtkfmt/ranges.h"
#include <vtkfmt/chrono.h>

#include "imgui.h"

enum class MyPicker : int { None, Point, Cell, Prop, };
struct ExampleAppLog
{
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int> LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
  bool AutoScroll;           // Keep scrolling if already at the bottom.

  ExampleAppLog()
  {
    AutoScroll = true;
    Clear();
  }

  void Clear()
  {
    Buf.clear();
    LineOffsets.clear();
    LineOffsets.push_back(0);
  }

  void Add(const std::string_view& log)
  {
    AddLog(fmt::format("{} {}\n", std::chrono::system_clock::now(), log).c_str());
  }

  void AddLog(const char* fmt, ...) IM_FMTARGS(2)
  {
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n')
        LineOffsets.push_back(old_size + 1);
  }

  void Draw()
  {
    if (ImGui::BeginChild("LogChild",
          ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
    {
      // Options menu
      if (ImGui::BeginPopup("Options"))
      {
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::EndPopup();
      }

      // Main window
      if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");
      ImGui::SameLine();
      bool clear = ImGui::Button("Clear");
      ImGui::SameLine();
      bool copy = ImGui::Button("Copy");
      ImGui::SameLine();
      Filter.Draw("Filter", -100.0f);

      ImGui::Separator();
      ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

      if (clear)
        Clear();
      if (copy)
        ImGui::LogToClipboard();

      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      const char* buf = Buf.begin();
      const char* buf_end = Buf.end();
      if (Filter.IsActive())
      {
        // In this example we don't use the clipper when Filter is enabled.
        // This is because we don't have random access to the result of our filter.
        // A real application processing logs with ten of thousands of entries may want to store the
        // result of search/filter.. especially if the filtering function is not trivial (e.g.
        // reg-exp).
        for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
        {
          const char* line_start = buf + LineOffsets[line_no];
          const char* line_end =
            (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
          if (Filter.PassFilter(line_start, line_end))
            ImGui::TextUnformatted(line_start, line_end);
        }
      }
      else
      {
        // The simplest and easy way to display the entire buffer:
        //   ImGui::TextUnformatted(buf_begin, buf_end);
        // And it'll just work. TextUnformatted() has specialization for large blob of text and will
        // fast-forward to skip non-visible lines. Here we instead demonstrate using the clipper to
        // only process lines that are within the visible area. If you have tens of thousands of
        // items and their processing cost is non-negligible, coarse clipping them on your side is
        // recommended. Using ImGuiListClipper requires
        // - A) random access into your data
        // - B) items all being the  same height,
        // both of which we can handle since we have an array pointing to the beginning of each line
        // of text. When using the filter (in the block of code above) we don't have random access
        // into the data to display anymore, which is why we don't use the clipper. Storing or
        // skimming through the search result would make it possible (and would be recommended if
        // you want to search through tens of thousands of entries).
        ImGuiListClipper clipper;
        clipper.Begin(LineOffsets.Size);
        while (clipper.Step())
        {
          for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
          {
            const char* line_start = buf + LineOffsets[line_no];
            const char* line_end =
              (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
            ImGui::TextUnformatted(line_start, line_end);
          }
        }
        clipper.End();
      }
      ImGui::PopStyleVar();

      if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

      ImGui::EndChild();
      ImGui::EndChild();
    }
  }
};
namespace Inj
{
	struct GData
	{
		vtkSmartPointer<vtkActor> actor;
		vtkSmartPointer<vtkPolyData> polyData;
		vtkSmartPointer<vtkCaptionRepresentation> captionRepresentation;
		vtkSmartPointer<vtkCaptionWidget> captionWidget;
		vtkSmartPointer<vtkCellPicker> InteractionPicker;
		vtkSmartPointer<vtkPropPicker> PropPicker;
		vtkSmartPointer<vtkProp3D> InteractionProp;
		vtkSmartPointer<vtkActor2D> lineActor;
		vtkNew<vtkPointPicker> pointPicker;
		vtkNew<vtkCellPicker> cellPicker;
		vtkNew<vtkPropPicker> propPicker;
		MyPicker myPickerType = MyPicker::None;
        ExampleAppLog log;
	};

	static void Pan(vtkRenderer* CurrentRenderer, vtkProp* InteractionProp)
	{
		fmt::print("---------------{}\n", __LINE__);
		if (CurrentRenderer == nullptr || InteractionProp == nullptr)
		{
			return;
		}

		vtkRenderWindowInteractor* rwi = CurrentRenderer->GetRenderWindow()->GetInteractor();

		if (auto prop = vtkActor2D::SafeDownCast(InteractionProp))
		{
			fmt::print("---------------{}\n", __LINE__);
		}
		return;
		// Use initial center as the origin from which to pan

		double* obj_center = nullptr;
		if (vtkProp3D::SafeDownCast(InteractionProp))
		{
			obj_center = vtkProp3D::SafeDownCast(InteractionProp)->GetCenter();
		}
		else if (auto prop = vtkActor2D::SafeDownCast(InteractionProp))
		{
			auto pos = prop->GetPosition();
			obj_center = new double[3] {pos[0], pos[1], 0 };
		}

		double disp_obj_center[3], new_pick_point[4];
		double old_pick_point[4], motion_vector[3];

		auto ComputeWorldToDisplay = [](vtkRenderer * ren, double x, double y, double z, double displayPt[3])
		{
			ren->SetWorldPoint(x, y, z, 1.0);
			ren->WorldToDisplay();
			ren->GetDisplayPoint(displayPt);
		};
		ComputeWorldToDisplay(CurrentRenderer, obj_center[0], obj_center[1], obj_center[2], disp_obj_center);

		auto ComputeDisplayToWorld = [](vtkRenderer* ren, double x, double y, double z, double worldPt[4])
			{
				ren->SetDisplayPoint(x, y, z);
				ren->DisplayToWorld();
				ren->GetWorldPoint(worldPt);
				if (worldPt[3])
				{
					worldPt[0] /= worldPt[3];
					worldPt[1] /= worldPt[3];
					worldPt[2] /= worldPt[3];
					worldPt[3] = 1.0;
				}
			};
		ComputeDisplayToWorld(
			CurrentRenderer, rwi->GetEventPosition()[0], rwi->GetEventPosition()[1], disp_obj_center[2], new_pick_point);

		ComputeDisplayToWorld(CurrentRenderer, rwi->GetLastEventPosition()[0], rwi->GetLastEventPosition()[1],
			disp_obj_center[2], old_pick_point);

		motion_vector[0] = new_pick_point[0] - old_pick_point[0];
		motion_vector[1] = new_pick_point[1] - old_pick_point[1];
		motion_vector[2] = new_pick_point[2] - old_pick_point[2];
#if 0
		if (InteractionProp->GetUserMatrix() != nullptr)
		{
			vtkTransform* t = vtkTransform::New();
			t->PostMultiply();
			t->SetMatrix(this->InteractionProp->GetUserMatrix());
			t->Translate(motion_vector[0], motion_vector[1], motion_vector[2]);
			this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
			t->Delete();
		}
		else
		{
			InteractionProp->AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
		}
#endif
		if (auto prop = vtkProp3D::SafeDownCast(InteractionProp))
		{
			prop->AddPosition(motion_vector[0], motion_vector[1], motion_vector[2]);
		}
		else if (auto prop = vtkActor2D::SafeDownCast(InteractionProp))
		{
			prop->SetPosition(prop->GetPosition()[0] + motion_vector[0], prop->GetPosition()[1] + motion_vector[1]);
		}
		//if (this->AutoAdjustCameraClippingRange)
		//{
		//	this->CurrentRenderer->ResetCameraClippingRange();
		//}

		rwi->Render();
	}
}

extern Inj::GData* gData;