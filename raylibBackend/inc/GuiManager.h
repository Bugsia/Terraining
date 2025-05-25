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

	void draw(Camera& camera);
	void addGui(Gui* gui);

private:
	std::vector<Gui*> m_gui;

	void defaultStyle();
};