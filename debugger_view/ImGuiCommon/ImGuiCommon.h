﻿#pragma once

#include "IncludeAllInOne.h"

template<typename T, std::size_t N>
using TmpArr_t = T[N];
#if 0
template<typename T, std::size_t N>
auto TmpArr = TmpArr_t<T, N>{}; // 不能为constexpr？
#else
template<typename T, typename = std::enable_if_t<std::is_array_v<T>>>
auto TmpArr = T{}; // 不能为constexpr？
#endif

namespace
{
	void setupDefaultVolumeProperty(vtkVolume* pVolume)
	{
		vtkNew<vtkVolumeProperty> pProperty;
		{
			vtkNew<vtkPiecewiseFunction> pOpacity;
			vtkNew<vtkColorTransferFunction> pColor;
			pProperty->ShadeOn();
			pProperty->SetAmbient(0.30);
			pProperty->SetDiffuse(0.50);
			pProperty->SetSpecular(0.25);
			pProperty->SetSpecularPower(37.5);
			pProperty->SetDisableGradientOpacity(1);

			pProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
			pProperty->SetScalarOpacity(pOpacity);
			pProperty->SetColor(pColor);
#if 0
			pOpacity->AddPoint(-50.0, 0.0);
			pOpacity->AddPoint(625.49, 0.0);
			pOpacity->AddPoint(1286.34, 0.0);
			pOpacity->AddPoint(1917.15, 0.70);
			pOpacity->AddPoint(2300, 1.0);
			pOpacity->AddPoint(4043.31, 1.0);
			pOpacity->AddPoint(5462.06, 1.0);

			pColor->AddRGBPoint(-50.38, 60 / 255., 0, 255 / 255.);
			pColor->AddRGBPoint(595.45, 91 / 255., 76 / 255., 141 / 255.);
			pColor->AddRGBPoint(1196.22, 170 / 255., 0 / 255., 0 / 255.);
			pColor->AddRGBPoint(1568.38, 208 / 255., 131 / 255., 79 / 255.);
			pColor->AddRGBPoint(2427.80, 235 / 255., 222 / 255., 133 / 255.);
			pColor->AddRGBPoint(2989.06, 255 / 255., 255 / 255., 255 / 255.);
			pColor->AddRGBPoint(4680.69, 1.0, 1.0, 1.0);
#else
			const std::list<std::tuple<double, double, std::array<double, 3>>> mylist
			{
				{-50.0, 0., {60. / 255., 0., 255. / 255.}},
				{625.49, 0., {91 / 255., 76 / 255., 141 / 255.}},
				{1286.34, 0., {170 / 255., 0 / 255., 0 / 255.}},
				{1917.15, 0.70, {208 / 255., 131 / 255., 79 / 255.}},
				{2300, 1.0, {235 / 255., 222 / 255., 133 / 255.}},
				{4043.31, 1.0, {255 / 255., 255 / 255., 255 / 255.}},
				{5462.06, 1.0, {255 / 255., 255 / 255., 0. / 255.}},
			};
			int i = 0;
			for (const auto [hu, op, rgb] : mylist)
			{
				pOpacity->AddPoint(hu, op);
				pProperty->GetIsoSurfaceValues()->SetValue(i++, hu);
				pColor->AddRGBPoint(hu, rgb[0], rgb[1], rgb[2]);
			}
#endif
		}
		pVolume->SetProperty(pProperty);
	}

	auto convertPointsToPolyline(vtkPoints* inputPoints)
	{
		vtkNew<vtkPolyLine> polyLine;
		polyLine->GetPointIds()->SetNumberOfIds(inputPoints->GetNumberOfPoints());
		for (unsigned int i = 0; i < inputPoints->GetNumberOfPoints(); i++)
		{
			polyLine->GetPointIds()->SetId(i, i);
		}

		// Create a cell array to store the lines in and add the lines to it
		vtkNew<vtkCellArray> cells;
		cells->InsertNextCell(polyLine);

		// Create a polydata to store everything in
		auto polyData = vtkSmartPointer<vtkPolyData>::New();

		// Add the points to the dataset
		polyData->SetPoints(inputPoints);

		// Add the lines to the dataset
		polyData->SetLines(cells);

		return polyData;
	}

	double GetCurveLength(vtkPoints* curvePoints, bool closedCurve, vtkIdType startCurvePointIndex, vtkIdType numberOfCurvePoints)
	{
		if (!curvePoints || curvePoints->GetNumberOfPoints() < 2)
		{
			return 0.0;
		}
		if (startCurvePointIndex < 0)
		{
			std::cout << "Invalid startCurvePointIndex=" << startCurvePointIndex << ", using 0 instead" << std::endl;

			startCurvePointIndex = 0;
		}
		vtkIdType lastCurvePointIndex = curvePoints->GetNumberOfPoints() - 1;
		if (numberOfCurvePoints >= 0 && startCurvePointIndex + numberOfCurvePoints - 1 < lastCurvePointIndex)
		{
			lastCurvePointIndex = startCurvePointIndex + numberOfCurvePoints - 1;
		}

		double length = 0.0;
		double previousPoint[3] = { 0.0 };
		double nextPoint[3] = { 0.0 };
		curvePoints->GetPoint(startCurvePointIndex, previousPoint);
		for (vtkIdType curvePointIndex = startCurvePointIndex + 1; curvePointIndex <= lastCurvePointIndex; curvePointIndex++)
		{
			curvePoints->GetPoint(curvePointIndex, nextPoint);
			length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
			previousPoint[0] = nextPoint[0];
			previousPoint[1] = nextPoint[1];
			previousPoint[2] = nextPoint[2];
		}
		// Add length of closing segment
		if (closedCurve && (numberOfCurvePoints < 0 || numberOfCurvePoints >= curvePoints->GetNumberOfPoints()))
		{
			curvePoints->GetPoint(0, nextPoint);
			length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
		}
		return length;
	}

	bool GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType& foundClosestPointIndex, vtkIdType startCurvePointId, double distanceFromStartPoint, vtkPoints* curvePoints, bool closedCurve)
	{
		vtkIdType numberOfCurvePoints = (curvePoints != nullptr ? curvePoints->GetNumberOfPoints() : 0);

		if (numberOfCurvePoints == 0)
		{
			std::cout << "AirwayInteractorStyle::GetPositionAndClosestPointIndexAlongCurve failed: invalid input points" << std::endl;
			foundClosestPointIndex = -1;
			return false;
		}
		if (startCurvePointId < 0 || startCurvePointId >= numberOfCurvePoints)
		{
			std::cout << "AirwayInteractorStyle::GetPositionAndClosestPointIndexAlongCurve failed: startCurvePointId is out of range" << std::endl;
			foundClosestPointIndex = -1;
			return false;
		}
		if (numberOfCurvePoints == 1 || distanceFromStartPoint == 0)
		{
			curvePoints->GetPoint(startCurvePointId, foundCurvePosition);
			foundClosestPointIndex = startCurvePointId;
			if (distanceFromStartPoint > 0.0)
			{
				std::cout << "AirwayInteractorStyle::GetPositionAndClosestPointIndexAlongCurve failed: non-zero distance"
					" is requested but only 1 point is available" << std::endl;
				return false;
			}
			else
			{
				return true;
			}
		}

		vtkIdType idIncrement = (distanceFromStartPoint > 0 ? 1 : -1);
		double remainingDistanceFromStartPoint = abs(distanceFromStartPoint);
		double previousPoint[3] = { 0.0 };
		curvePoints->GetPoint(startCurvePointId, previousPoint);
		vtkIdType pointId = startCurvePointId;
		bool curveConfirmedToBeNonZeroLength = false;
		double lastSegmentLength = 0;

		while (true)
		{
			pointId += idIncrement;

			// if reach the end then wrap around for closed curve, terminate search for open curve
			if (pointId < 0 || pointId >= numberOfCurvePoints)
			{
				if (closedCurve)
				{
					if (!curveConfirmedToBeNonZeroLength)
					{
						if (GetCurveLength(curvePoints, closedCurve, -1, numberOfCurvePoints) == 0.0)
						{
							foundClosestPointIndex = -1;
							return false;
						}
						curveConfirmedToBeNonZeroLength = true;
					}
					pointId = (pointId < 0 ? numberOfCurvePoints : -1);
					continue;
				}
				else
				{
					// reached end of curve before getting at the requested distance
					// return closest
					foundClosestPointIndex = (pointId < 0 ? 0 : numberOfCurvePoints - 1);
					curvePoints->GetPoint(foundClosestPointIndex, foundCurvePosition);
					return false;
				}
			}

			// determine how much closer we are now
			double* nextPoint = curvePoints->GetPoint(pointId);
			lastSegmentLength = sqrt(vtkMath::Distance2BetweenPoints(nextPoint, previousPoint));
			remainingDistanceFromStartPoint -= lastSegmentLength;

			if (remainingDistanceFromStartPoint <= 0)
			{
				// reached the requested distance (and probably a bit more)
				for (int i = 0; i < 3; i++)
				{
					foundCurvePosition[i] = nextPoint[i] +
						remainingDistanceFromStartPoint * (nextPoint[i] - previousPoint[i]) / lastSegmentLength;
				}
				if (fabs(remainingDistanceFromStartPoint) <= fabs(remainingDistanceFromStartPoint + lastSegmentLength))
				{
					foundClosestPointIndex = pointId;
				}
				else
				{
					foundClosestPointIndex = pointId - 1;
				}
				break;
			}

			previousPoint[0] = nextPoint[0];
			previousPoint[1] = nextPoint[1];
			previousPoint[2] = nextPoint[2];
		}
		return true;
	}
	bool ResamplePoints(vtkPoints* originalPoints, vtkPoints* sampledPoints, double samplingDistance, bool closedCurve)
	{
		if (!originalPoints || !sampledPoints || samplingDistance <= 0)
		{
			vtkGenericWarningMacro("AirwayInteractorStyle::ResamplePoints failed: invalid inputs");

			return false;
		}

		if (originalPoints->GetNumberOfPoints() < 2)
		{
			sampledPoints->DeepCopy(originalPoints);

			return true;
		}

		double distanceFromLastSampledPoint = 0;
		double remainingSegmentLength = 0;
		double previousCurvePoint[3] = { 0.0 };
		originalPoints->GetPoint(0, previousCurvePoint);
		sampledPoints->Reset();
		sampledPoints->InsertNextPoint(previousCurvePoint);

		vtkIdType numberOfOriginalPoints = originalPoints->GetNumberOfPoints();
		bool addClosingSegment = closedCurve; // for closed curves, add a closing segment that connects last and first points
		double* currentCurvePoint = nullptr;
		for (vtkIdType originalPointIndex = 0; originalPointIndex < numberOfOriginalPoints || addClosingSegment; originalPointIndex++)
		{
			if (originalPointIndex >= numberOfOriginalPoints)
			{
				// this is the closing segment
				addClosingSegment = false;
				currentCurvePoint = originalPoints->GetPoint(0);
			}
			else
			{
				currentCurvePoint = originalPoints->GetPoint(originalPointIndex);
			}

			double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(currentCurvePoint, previousCurvePoint));
			if (segmentLength <= 0.0)
			{
				continue;
			}
			remainingSegmentLength = distanceFromLastSampledPoint + segmentLength;
			if (remainingSegmentLength >= samplingDistance)
			{
				double segmentDirectionVector[3] =
				{
					(currentCurvePoint[0] - previousCurvePoint[0]) / segmentLength,
					(currentCurvePoint[1] - previousCurvePoint[1]) / segmentLength,
					(currentCurvePoint[2] - previousCurvePoint[2]) / segmentLength
				};
				// distance of new sampled point from previous curve point
				double distanceFromLastInterpolatedPoint = samplingDistance - distanceFromLastSampledPoint;
				while (remainingSegmentLength >= samplingDistance)
				{
					double newSampledPoint[3] =
					{
						previousCurvePoint[0] + segmentDirectionVector[0] * distanceFromLastInterpolatedPoint,
						previousCurvePoint[1] + segmentDirectionVector[1] * distanceFromLastInterpolatedPoint,
						previousCurvePoint[2] + segmentDirectionVector[2] * distanceFromLastInterpolatedPoint
					};
					sampledPoints->InsertNextPoint(newSampledPoint);
					distanceFromLastSampledPoint = 0;
					distanceFromLastInterpolatedPoint += samplingDistance;

					remainingSegmentLength -= samplingDistance;
				}
				distanceFromLastSampledPoint = remainingSegmentLength;
			}
			else
			{
				distanceFromLastSampledPoint += segmentLength;
			}
			previousCurvePoint[0] = currentCurvePoint[0];
			previousCurvePoint[1] = currentCurvePoint[1];
			previousCurvePoint[2] = currentCurvePoint[2];
		}

		// Make sure the resampled curve has the same size as the original
		// but avoid having very long or very short line segments at the end.
		if (closedCurve)
		{
			// Closed curve
			// Ideally, remainingSegmentLength would be equal to samplingDistance.
			if (remainingSegmentLength < samplingDistance * 0.5)
			{
				// last segment would be too short, so remove the last point and adjust position of second last point
				double lastPointPosition[3] = { 0.0 };
				vtkIdType lastPointOriginalPointIndex = 0;
				if (GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
					0, -(2.0 * samplingDistance + remainingSegmentLength) / 2.0, originalPoints, closedCurve))
				{
					sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
					sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
				}
				else
				{
					// something went wrong, we could not add a point, therefore just remove the last point
					sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
				}
			}
			else
			{
				// last segment is only slightly shorter than the sampling distance
				// so just adjust the position of the last point
				double lastPointPosition[3] = { 0.0 };
				vtkIdType lastPointOriginalPointIndex = 0;
				if (GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, lastPointOriginalPointIndex,
					0, -(samplingDistance + remainingSegmentLength) / 2.0, originalPoints, closedCurve))
				{
					sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
				}
			}
		}
		else
		{
			// Open curve
			// Ideally, remainingSegmentLength would be equal to 0.
			if (remainingSegmentLength > samplingDistance * 0.5)
			{
				// last segment would be much longer than the sampling distance, so add an extra point
				double secondLastPointPosition[3] = { 0.0 };
				vtkIdType secondLastPointOriginalPointIndex = 0;
				if (GetPositionAndClosestPointIndexAlongCurve(secondLastPointPosition, secondLastPointOriginalPointIndex,
					originalPoints->GetNumberOfPoints() - 1, -(samplingDistance + remainingSegmentLength) / 2.0, originalPoints, closedCurve))
				{
					sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, secondLastPointPosition);
					sampledPoints->InsertNextPoint(originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
				}
				else
				{
					// something went wrong, we could not add a point, therefore just adjust the last point position
					sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
						originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
				}
			}
			else
			{
				// last segment is only slightly longer than the sampling distance
				// so we just adjust the position of last point
				sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
					originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
			}
		}

		return true;
	}
}

namespace vtkns
{
	static auto getMatrixString(vtkMatrix4x4* obj)
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
}

namespace vtkns
{
    void HelpMarker(const char* desc);
	void HelpMarkerSameLine(const char* desc);
    void vtkObjSetup(std::string_view objName, vtkSmartPointer<vtkObject> vtkObj, const ImGuiTreeNodeFlags = 0);
	inline void vtkObjSetupWin(std::string_view objName, vtkSmartPointer<vtkObject> vtkObj)
	{
		static std::map<const void*, bool> mymap; // 两个内容相同的字符串字面量的内存地址一定不同吗？
		ImGui::Checkbox(objName.data(), &mymap[objName.data()]);
		if (mymap[objName.data()])
		{
			ImGui::Begin(objName.data());
			vtkns::vtkObjSetup("", vtkObj, ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::End();
		}
	}
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

		template <typename... Ts>
		void Add(Ts&&... args)
		{
			auto log = fmt::format(std::forward<Ts>(args)...);

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

	void printWorldPt(vtkns::LogView& logView, vtkRenderer* pRenderer, double disPtX, double disPtY);
}

static std::function<void()> imgui_render_callback;
static vtkWeakPointer<vtkRenderWindow> pWindow;
inline vtkWeakPointer<vtkRenderer> pRen;
static bool showLogView = false;
static vtkns::LogView* logger()
{
	static vtkns::LogView o;
	return &o;
}

namespace vtkns
{
	template <typename... Ts>
	void log(Ts&&... args)
	{
		::logger()->Add(std::forward<Ts>(args)...);
	}
}

namespace vtkns
{
	static void DrawUI(vtkDearImGuiInjector* overlay)
	{
		vtkNew<vtkCallbackCommand> uiDraw;
		auto uiDrawFunction = [](vtkObject* caller, long unsigned int vtkNotUsed(eventId),
			void* clientData, void* vtkNotUsed(callData))
		{
			vtkDearImGuiInjector* overlay_ = reinterpret_cast<vtkDearImGuiInjector*>(caller);

			ImGui::SetNextWindowPos(ImVec2(0, 25), ImGuiCond_Once);
			// ImGui::SetNextWindowSize(ImVec2(450, 650), ImGuiCond_Once);
			ImGui::Begin("VTK");
			{
				ImGui::Checkbox("ShowLogView", &::showLogView);
				ImGui::SameLine();
				if (ImGui::Button(u8"距离测量"))
				{
					//vtkNew<vtkDistanceWidget> distanceWidget;
					auto distanceWidget = vtkDistanceWidget::New();
					{
						distanceWidget->SetInteractor(::pWindow->GetInteractor());
						distanceWidget->CreateDefaultRepresentation();
						vtkDistanceRepresentation::SafeDownCast(distanceWidget->GetRepresentation())->SetLabelFormat("%-#6.3g mm");
						::pWindow->Render();
						distanceWidget->On();
					}
				}

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
							vtkns::vtkObjSetup("Camera", ::pWindow->GetRenderers()->GetFirstRenderer()->GetActiveCamera(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("RendererInfo"))
						{
							vtkns::vtkObjSetup("Renderer", ::pWindow->GetRenderers()->GetFirstRenderer(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("InteractorInfo"))
						{
							vtkns::vtkObjSetup("Interactor", ::pWindow->GetInteractor(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("WindowInfo"))
						{
							vtkns::vtkObjSetup("Window", ::pWindow.Get(), ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("InteractorStyle"))
						{
							vtkns::vtkObjSetup("Style", ::pWindow->GetInteractor()->GetInteractorStyle(), ImGuiTreeNodeFlags_DefaultOpen);
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

			if (::showLogView)
			{
				ImGui::Begin("Log");
				::logger()->Draw();
				ImGui::End();
			}
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
				io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/simhei.ttf", 15.8f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
			}
		};
		uiSetup->SetCallback(uiSetupFunction);
		overlay->AddObserver(vtkDearImGuiInjector::ImGuiSetupEvent, uiSetup);
	}

	static const char* getDicomFile()
	{
		const char* retval = "D:/test_data/series/I0000000200.dcm";
		//const char* retval = "C:\\Users\\123\\Desktop\\series\\I0000000200.dcm";
		if (!std::filesystem::exists(retval))
		{
			throw "dicom file does not exist!";
		}
		return retval;
	}

	static const char* getDicomDir()
	{
		const char* retval = "D:/test_data/series";
		//const char* retval = "C:\\Users\\123\\Desktop\\series";
		if (!std::filesystem::exists(retval))
		{
			throw "dicom dir does not exist!";
		}
		return retval;
	}

	static vtkSmartPointer<vtkImageData> getVRData()
	{
		vtkNew<vtkDICOMImageReader> reader;
		reader->SetDirectoryName(vtkns::getDicomDir());
		reader->Update();
		return reader->GetOutput();
	}

	static vtkSmartPointer<vtkImageData> getSliceData()
	{
		vtkNew<vtkDICOMImageReader> reader;
		reader->SetFileName(vtkns::getDicomFile());
		reader->Update();
		return reader->GetOutput();
	}

	static void labelWorldZero(vtkRenderer* pRen, const bool b = true, const double radius = 1., const double len = 10.)
	{
		// 在世界原点放置一个标记
		if (b)
		{
			vtkNew<vtkSphereSource> pointSource;
			pointSource->SetCenter(0, 0, 0);
			//pointSource->SetNumberOfPoints(1);
			pointSource->SetRadius(radius);
			pointSource->Update();

			// Create a mapper and actor.
			vtkNew<vtkPolyDataMapper> mapper;
			mapper->SetInputConnection(pointSource->GetOutputPort());

			vtkNew<vtkActor> actor;
			actor->SetMapper(mapper);
			actor->GetProperty()->SetColor(1, 0.5, 0);
			actor->GetProperty()->SetPointSize(5);
			pRen->AddActor(actor);
		}
		// 画三条线表示XYZ轴
		{
			struct
			{
				double pos[3];
				double color[3];
			} xLine{ {len, 0, 0},{1,0,0} }, yLine{ {0,len,0},{0,1,0} }, zLine{ {0,0,len}, {0,0,1} };

			for (const auto& [pos, color] : { xLine, yLine, zLine })
			{
				vtkNew<vtkLineSource> pSource;
				pSource->SetPoint1(0, 0, 0);;
				pSource->SetPoint2(pos);
				pSource->Update();

				vtkNew<vtkPolyDataMapper> pMapper;
				pMapper->SetInputConnection(pSource->GetOutputPort());

				vtkNew<vtkActor> pActor;
				pActor->SetMapper(pMapper);
				pActor->GetProperty()->SetColor(const_cast<double*>(color));
				pActor->GetProperty()->SetPointSize(5);
				pRen->AddActor(pActor);
			}
		}
	}
	static void genImgMesh(vtkRenderer* ren, vtkAlgorithmOutput* pData)
	{
		vtkNew<vtkDataSetMapper> pMapper;
		pMapper->SetInputConnection(pData);

		vtkNew<vtkActor> actor;
		actor->GetProperty()->SetRepresentationToWireframe();
		// actor->GetProperty()->SetColor(0, 1, 0);
		actor->GetProperty()->SetOpacity(.5);
		actor->SetMapper(pMapper);
		ren->AddActor(actor);
	}

#if 1 // 后续合并两个版本函数
	static void genImgOutline(vtkRenderer* ren, vtkAlgorithmOutput* pData)
	{
		vtkNew<vtkImageDataOutlineFilter> pFilter;
		pFilter->SetInputConnection(pData);

		vtkNew<vtkPolyDataMapper> pMapper;
		pMapper->SetInputConnection(pFilter->GetOutputPort());

		vtkNew<vtkActor> actor;
		actor->SetMapper(pMapper);
		ren->AddActor(actor);
	}

	static void updateImgOutlineMapper_(vtkImageData* pData, vtkActor* actor)
	{
		vtkNew<vtkImageDataOutlineFilter> pFilter;
		pFilter->SetInputData(pData);

		vtkNew<vtkPolyDataMapper> pMapper;
		pMapper->SetInputConnection(pFilter->GetOutputPort());

		actor->SetMapper(pMapper);
	}

	// 在指定的renderer上添加一个显示指定img轮廓的actor，并可指定是否监控img的变化动态显示轮廓
	static auto genImgOutline(vtkRenderer* pRenderer, vtkImageData* pData, const bool b)
	{
		vtkNew<vtkActor> pActor;
		pRenderer->AddActor(pActor);

		if (b)
		{
			auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
				{
					auto pActor = reinterpret_cast<vtkActor*>(clientdata);
					updateImgOutlineMapper_(vtkImageData::SafeDownCast(caller), pActor);
				};
			vtkNew<vtkCallbackCommand> pCC;
			pCC->SetCallback(f);
			pCC->SetClientData(pActor);
			pData->AddObserver(vtkCommand::ModifiedEvent, pCC);
		}
		else
		{
			updateImgOutlineMapper_(pData, pActor);
		}

		return pActor;
	}
#endif
#if 1 // 后续合并两个版本的函数
	static void genVR_(vtkVolume* volume, vtkImageData* data, const bool lowOpacity)
	{
		vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
		pMapper->SetInputData(data);
		pMapper->SetBlendModeToComposite();
		volume->SetMapper(pMapper);
		::setupDefaultVolumeProperty(volume);
	}
	static void genVR_(vtkVolume* volume, vtkAlgorithmOutput* data, const bool lowOpacity)
	{
		vtkNew<vtkGPUVolumeRayCastMapper> pMapper;
		pMapper->SetInputConnection(data);
		pMapper->SetBlendModeToComposite();
		volume->SetMapper(pMapper);
		::setupDefaultVolumeProperty(volume);
	}

	static auto genVR(vtkRenderer* ren, vtkImageData* pImg, const bool onChange = true, const bool lowOpacity = false)
	{
		vtkNew<vtkVolume> pVolume;
		ren->AddVolume(pVolume);
#if 0 // 不需要此代码
		if (onChange)
		{
			auto f = [](vtkObject* caller, unsigned long eid, void* clientdata, void* calldata)
			{
				auto pActor = reinterpret_cast<vtkVolume*>(((void**)clientdata)[0]);
				vtkGPUVolumeRayCastMapper::SafeDownCast(pActor->GetMapper())->SetInputData(vtkImageData::SafeDownCast(caller));
				//pActor->GetMapper()->SetInputDataObject(vtkImageData::SafeDownCast(caller));
				//const bool lowOpacity = *reinterpret_cast<bool*>(((void**)clientdata)[1]);
				//genVR_(pActor, vtkImageData::SafeDownCast(caller), lowOpacity);
			};
			vtkNew<vtkCallbackCommand> pCC;
			pCC->SetCallback(f);
			void** clientData = new void* [2]; // 内存泄露
			clientData[0] = pVolume.Get();
			clientData[1] = new bool{ lowOpacity };
			pCC->SetClientData(clientData);
			pImg->AddObserver(vtkCommand::ModifiedEvent, pCC);
		}
#endif
		genVR_(pVolume, pImg, lowOpacity);
		if (lowOpacity)
		{
			// 调节透明度让slice更清楚
			pVolume->GetProperty()->GetScalarOpacity()->RemoveAllPoints();
			pVolume->GetProperty()->GetScalarOpacity()->AddPoint(1000, 0.);
			pVolume->GetProperty()->GetScalarOpacity()->AddPoint(5000, 0.1);
		}

		return pVolume;
	}
	static auto genVR(vtkRenderer* ren, vtkAlgorithmOutput* pImg, const bool onChange = true, const bool lowOpacity = false)
	{
		vtkNew<vtkVolume> pVolume;
		ren->AddVolume(pVolume);
		genVR_(pVolume, pImg, lowOpacity);
		if (lowOpacity)
		{
			// 调节透明度让slice更清楚
			pVolume->GetProperty()->GetScalarOpacity()->RemoveAllPoints();
			pVolume->GetProperty()->GetScalarOpacity()->AddPoint(1000, 0.);
			pVolume->GetProperty()->GetScalarOpacity()->AddPoint(5000, 0.1);
		}

		return pVolume;
	}
#endif
	static auto genPolyDataActor(vtkRenderer* ren, vtkAlgorithmOutput* algoOutput)
	{
		vtkNew<vtkPolyDataMapper> mapper;
		mapper->SetInputConnection(algoOutput);

		vtkNew<vtkActor> actor;
		ren->AddActor(actor);
		actor->SetMapper(mapper);

		return actor;
	}

	static auto getX()
	{
		auto ren = vtkSmartPointer<vtkRenderer>::New();
		// ren->SetBackground(0.2, 0.3, 0.4);
		ren->SetBackground(0., 0., 0.);
		auto renWin = vtkSmartPointer<vtkRenderWindow>::New();
		renWin->AddRenderer(ren);
		auto iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
		iren->SetRenderWindow(renWin);
		vtkInteractorStyleSwitch::SafeDownCast(iren->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();

		::pWindow = renWin;
		::pRen = ren;

		// 相机方向
		auto camManipulator = vtkSmartPointer<vtkCameraOrientationWidget>::New();
		camManipulator->SetParentRenderer(ren);
		camManipulator->On();
		auto rep = vtkCameraOrientationRepresentation::SafeDownCast(camManipulator->GetRepresentation());
		rep->AnchorToLowerRight();

		return std::make_tuple(renWin, ren, iren, camManipulator);
	}
#define BEFORE_MY_CODE auto [rw, ren, rwi, camManipulator] = vtkns::getX();

	static auto after_my_code_func = [](auto&& rw, auto&& rwi)
	{
		// Start rendering app
		rw->Render(); // 非常重要！！

		/// Change to your code begins here. ///
		// Initialize an overlay with DearImgui elements.
		vtkNew<vtkDearImGuiInjector> dearImGuiOverlay;
		// 💉 the overlay.
		dearImGuiOverlay->Inject(rwi);
		// These functions add callbacks to ImGuiSetupEvent and ImGuiDrawEvents.
		vtkns::SetupUI(dearImGuiOverlay);
		// You can draw custom user interface elements using ImGui:: namespace.
		vtkns::DrawUI(dearImGuiOverlay);
		/// Change to your code ends here. ///

		// Start event loop
#if 0
		renderWindow->SetSize(1920, 1000);
#else
#ifdef _WIN32
		// 获取窗口句柄
		HWND hwnd = ::FindWindow(NULL, rw->GetWindowName());
		// 最大化窗口
		::ShowWindow(hwnd, SW_MAXIMIZE);
#endif
#endif
		rwi->EnableRenderOff();
		rwi->Start();
	};
#define AFTER_MY_CODE vtkns::after_my_code_func(rw, rwi);
}