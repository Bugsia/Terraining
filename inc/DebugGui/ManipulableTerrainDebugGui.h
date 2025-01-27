#pragma once
#include <raylib.h>
#include "Gui.h"
#include "Terrain/TerrainManager.h"

namespace DebugGui {
	class ManipulableTerrainDebugGui : public Gui {
	public:
		ManipulableTerrainDebugGui(std::string name, Terrain::TerrainManager& terrain, Camera& camera);

		bool render();

	private:
		Terrain::TerrainManager& m_terrain;
		Ray m_mouseRay = { 0 };
		RayCollision m_mouseCollision = { 0 };
		Camera& m_camera;

		bool m_checkMouse = false;

		void updateMouseInformation();
		void renderMouseInformation();
		void renderManualManipulation();
	};
}