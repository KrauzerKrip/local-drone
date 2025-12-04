#pragma once

#include <imgui.h>
#include <filesystem>

struct ImGuiFonts {
	ImFont* m_pFontTitles;
	ImFont* m_pFontText;

	ImGuiFonts(std::filesystem::path resourceDir) {
		auto io = ImGui::GetIO();
		m_pFontTitles = io.Fonts->AddFontFromFileTTF((resourceDir / "dev/fonts/IBM_Plex_Mono/IBMPlexMono-Regular.ttf").c_str(), 36.0f);
		m_pFontText = io.Fonts->AddFontFromFileTTF((resourceDir / "dev/fonts/IBM_Plex_Mono/IBMPlexMono-Regular.ttf").c_str(), 24.0f);
	}
};
