#pragma once

#include <string_view>

#include <imgui.h>

namespace vtkns
{
    template <typename... Ts>
    void ImGuiText(Ts&&... args)
    {
        ImGui::Text(fmt::format(std::forward<Ts>(args)...).c_str());
    }

    // 不必考虑是否多次pushrepeat了
	inline void ArrowButton(const char* title, std::function<void()> fl, std::function<void()> fr) // inline??
	{
        // Use AlignTextToFramePadding() to align text baseline to the baseline of framed widgets elements
        // (otherwise a Text+SameLine+Button sequence will have the text a little too high by default!)
        // See 'Demo->Layout->Text Baseline Alignment' for details.
        ImGui::AlignTextToFramePadding();
        vtkns::ImGuiText("{}:", title);
        ImGui::SameLine();

        // Arrow buttons with Repeater
        ImGui::PushButtonRepeat(true);
        {
            ImGui::PushID(title);
            if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { fl(); }
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { fr(); }
            ImGui::PopID();
        }
        ImGui::PopButtonRepeat();
        //ImGui::SameLine();
        //ImGui::Text("%d", counter);
	}

    inline void ArrowButtonSameLine()
    {
        ImGui::SameLine(0.0f, 3 * ImGui::GetStyle().ItemInnerSpacing.x);
    }
}