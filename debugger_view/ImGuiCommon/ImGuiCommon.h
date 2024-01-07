#pragma once

#include "IncludeAllInOne.h"
#include "ImageSharpenFilter.h"
#include "DrawFreeLineStyle.h"

namespace ImGuiNs
{
    void HelpMarker(const char* desc);
    void vtkObjSetup(std::string_view objName, vtkSmartPointer<vtkObject> vtkObj, const ImGuiTreeNodeFlags = 0);
	struct LogView
	{
		ImGuiTextBuffer     Buf;
		ImGuiTextFilter     Filter;
		ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
		bool                AutoScroll;  // Keep scrolling if already at the bottom.
		bool				ShowTimestamp = false;

		LogView()
		{
			AutoScroll = true;
			Clear();
		}

		void    Clear()
		{
			Buf.clear();
			LineOffsets.clear();
			LineOffsets.push_back(0);
		}

		void Add(const std::string_view& log)
		{
			if (this->ShowTimestamp)
			{
				AddLog(fmt::format("{} {}\n", std::chrono::system_clock::now(), log).c_str());
			}
			else
			{
				AddLog(fmt::format("{}\n", log).c_str());
			}
		}

		void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
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

		void    Draw()
		{
			// Options menu
			if (ImGui::BeginPopup("Options"))
			{
				ImGui::Checkbox("Auto-scroll", &AutoScroll);
				ImGui::Checkbox("Timestamp", &this->ShowTimestamp);
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
				// A real application processing logs with ten of thousands of entries may want to store the result of
				// search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
				for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					if (Filter.PassFilter(line_start, line_end))
						ImGui::TextUnformatted(line_start, line_end);
				}
			}
			else
			{
				// The simplest and easy way to display the entire buffer:
				//   ImGui::TextUnformatted(buf_begin, buf_end);
				// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
				// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
				// within the visible area.
				// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
				// on your side is recommended. Using ImGuiListClipper requires
				// - A) random access into your data
				// - B) items all being the  same height,
				// both of which we can handle since we have an array pointing to the beginning of each line of text.
				// When using the filter (in the block of code above) we don't have random access into the data to display
				// anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
				// it possible (and would be recommended if you want to search through tens of thousands of entries).
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step())
				{
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
					{
						const char* line_start = buf + LineOffsets[line_no];
						const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();

			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndChild();
		}
	};

	void printWorldPt(ImGuiNs::LogView& logView, vtkRenderer* pRenderer, double disPtX, double disPtY);
}

static std::function<void()> imgui_render_callback;
static vtkWeakPointer<vtkRenderWindow> pWindow;

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
			ImGui::SetNextWindowSize(ImVec2(450, 650), ImGuiCond_Once);
			ImGui::Begin("VTK");
			{
				if (::pWindow.Get())
				{
					if (ImGui::BeginTabBar("MyTabBar666"))
					{
						if (ImGui::BeginTabItem("Obj"))
						{
							::imgui_render_callback();
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("CameraInfo"))
						{
							ImGuiNs::vtkObjSetup("Camera", ::pWindow->GetRenderers()->GetFirstRenderer()->GetActiveCamera(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("RendererInfo"))
						{
							ImGuiNs::vtkObjSetup("Renderer", ::pWindow->GetRenderers()->GetFirstRenderer(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("InteractorInfo"))
						{
							ImGuiNs::vtkObjSetup("Interactor", ::pWindow->GetInteractor(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("WindowInfo"))
						{
							ImGuiNs::vtkObjSetup("Window", ::pWindow.Get(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("InteractorStyle"))
						{
							ImGuiNs::vtkObjSetup("Style", ::pWindow->GetInteractor()->GetInteractorStyle(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}
				}
				else
				{
					//if (::imgui_render_callback)
					{
						::imgui_render_callback();
					}
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
		//const char* retval = "D:/test_data/series/I0000000200.dcm";
		const char* retval = "C:\\Users\\123\\Desktop\\series\\I0000000200.dcm";
		if (!std::filesystem::exists(retval))
		{
			throw "dicom file does not exist!";
		}
		return retval;
	}

	static const char* getDicomDir()
	{
		//const char* retval = "D:/test_data/series";
		const char* retval = "C:\\Users\\123\\Desktop\\series";
		if (!std::filesystem::exists(retval))
		{
			throw "dicom dir does not exist!";
		}
		return retval;
	}
}