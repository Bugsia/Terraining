#include "DebugGui/TerrainDebugGui.h"

namespace DebugGui {
	TerrainDebugGui::TerrainDebugGui(std::string name, Terrain::TerrainManager& terrain, GuiManager& guiManager) : Gui(name), m_terrain(terrain), m_settings(*m_terrain.refSettings()), m_guiManager(guiManager) {}

	bool TerrainDebugGui::render() {
		ImGui::Begin(m_name.c_str(), &m_open);
		defaultStyle();

		if (ImGui::SliderFloat("Spawn Distance", &m_settings.radius, 1.0f, 1000.0f)) m_simpleChange = true;
		if (ImGui::SliderInt("Max Elements", (int*)&m_settings.maxNumElements, 1, 1000)) m_simpleChange = true;

		ImGui::SeparatorText("Terrain Element Settings");
		if (ImGui::InputInt("#Width", &m_settings.numWidth)) m_complexChange = true;
		if (ImGui::InputInt("#Height", &m_settings.numHeight)) m_complexChange = true;
		if (ImGui::SliderFloat("Spacing", &m_settings.spacing, 0.1f, 10.0f)) m_complexChange = true;
		if (ImGui::Button("Open Noise Settings") && !m_openNoiseGui) {
			m_openNoiseGui = true;
			m_guiManager.addGui(std::make_unique<NoiseDebugGui>(NoiseDebugGui("" + m_name + " Noise", m_terrain)));
		}

		ImGui::SeparatorText("Drawing Settings");
		ImGui::Checkbox("Wireframe", &m_settings.drawWireframe);
		ImGui::Checkbox("Normals", &m_settings.drawNormals);
		ImGui::SliderFloat("Terrain Model Scale", &m_settings.scale, 0.1f, 10.0f);
		ImGui::ColorEdit4("Tint", (float*)&m_settings.tint);

		ImGui::SeparatorText("");
		if (ImGui::Button("Apply")) {
			float oldRadius = m_terrain.refSettings()->radius;
			(*m_terrain.refSettings()) = m_settings;
			if (m_complexChange) m_terrain.renewTerrain();
			else if (m_simpleChange) m_terrain.updateTerrain(oldRadius);

			m_simpleChange = false;
			m_complexChange = false;
		}

		ImGui::End();

		return m_open;
	}
}