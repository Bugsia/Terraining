#pragma once
#include <memory>
#include "Gui.h"
#include "Terrain/TerrainManager.h"
#include "DebugGui/NoiseDebugGui.h"
#include "imgui.h"
#include "GuiManager.h"

namespace DebugGui {
	class TerrainDebugGui : public Gui {
	public:
		TerrainDebugGui(std::string name, Terrain::TerrainManager& terrain, GuiManager& guiManager);

		bool render();

	private:
		Terrain::TerrainManager& m_terrain;
		GuiManager& m_guiManager;
		Terrain::terrain_settings m_settings;
		bool m_openNoiseGui = false;
		bool m_simpleChange = false;
		bool m_complexChange = false;
		bool m_drawWired;
		bool m_drawNormals;
		float m_scale;
		Color m_tint;
	};
}