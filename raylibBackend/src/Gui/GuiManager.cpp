#include "GuiManager.h"

GuiManager::GuiManager(bool darkMode) {
	rlImGuiSetup(darkMode);
}

GuiManager::~GuiManager() {
	rlImGuiShutdown();
}

void GuiManager::draw(Camera& camera) {
	rlImGuiBegin();

	for (int i = 0; i < m_gui.size(); i++) {
		if (!m_gui[i]->render()) {
			m_gui.erase(m_gui.begin() + i);
			i--; // Otherwise a gui gets skipped
		}
	}

	rlImGuiEnd();
}

void GuiManager::addGui(Gui* gui) {
	m_gui.push_back(std::move(gui));
}