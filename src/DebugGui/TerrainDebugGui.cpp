#include "DebugGui/TerrainDebugGui.h"

namespace DebugGui {
	TerrainDebugGui::TerrainDebugGui(std::string name, Terrain::TerrainManager& terrain, GuiManager& guiManager) : Gui(name), m_terrain(terrain), m_settings(*m_terrain.refSettings()), m_guiManager(guiManager), m_drawWired(m_terrain.getDrawWired()), m_drawNormals(m_terrain.getDrawNormals()), m_scale(m_terrain.getScale()), m_tint(m_terrain.getTint()) {}

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
			m_guiManager.addGui(std::make_unique<NoiseDebugGui>(NoiseDebugGui("" + m_name + " Noise", m_terrain, &m_openNoiseGui)));
		}

		ImGui::SeparatorText("Drawing Settings (Instant)");
		if (ImGui::Checkbox("Wireframe", &m_drawWired)) m_terrain.setDrawWired(m_drawWired);
		if(ImGui::Checkbox("Normals", &m_drawNormals)) m_terrain.setDrawNormals(m_drawNormals);
		if (ImGui::SliderFloat("Terrain Model Scale", &m_scale, 0.1f, 10.0f)) m_terrain.setScale(m_scale);
		if (ImGui::ColorEdit4("Tint", (float*)&m_tint)) m_terrain.setTint(m_tint);

		ImGui::SeparatorText("MISC. (Instant)");
		if (ImGui::Checkbox("Follow Camera", &m_settings.followCamera)) m_settingsChange = true;
		if (ImGui::Checkbox("Update with ThreadPool", &m_settings.updateWithThreadPool)) m_settingsChange = true;

		if (m_settingsChange) {
			(*m_terrain.refSettings()) = m_settings;
			m_settingsChange = false;
		}

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