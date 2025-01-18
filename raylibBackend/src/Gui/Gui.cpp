#include "Gui.h"

Gui::Gui() : m_name("Gui"), m_open(true) {}

Gui::Gui(std::string name) : m_name(name), m_open(true) {}

bool Gui::isOpen() {
	return m_open;
}

void Gui::isOpen(bool open) {
	m_open = open;
}

void Gui::defaultStyle() {
	ImGui::SetWindowFontScale(2.0f);
	ImGui::PushItemWidth(200.0f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing.y = 10.0f;
}