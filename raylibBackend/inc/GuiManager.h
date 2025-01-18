#pragma once
#include <vector>
#include <memory>
#include "Gui.h"
#include "Drawable.h"
#include "rlImGui.h"
#include "imgui.h"

class GuiManager : Drawable {
public:
	GuiManager(bool darkMode);
	~GuiManager();

	void draw();
	void addGui(std::unique_ptr<Gui> gui);

private:
	std::vector<std::unique_ptr<Gui>> m_gui;

	void defaultStyle();
};