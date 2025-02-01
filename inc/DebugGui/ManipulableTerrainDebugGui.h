#pragma once
#include <raylib.h>
#include "Gui.h"
#include "Terrain/TerrainManager.h"
#include "Terrain/ManipulableTerrainElement.h"

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
		Vector3 m_manipulationPosition = { 0, 0, 0 };

		// Manipulation
		bool m_checkMouse = false;
		Terrain::ManipulableTerrainElement::ManipulateDir m_manipulateDir = Terrain::ManipulableTerrainElement::ManipulateDir::Y;
		Terrain::ManipulableTerrainElement::ManipulateForm m_manipulateForm = Terrain::ManipulableTerrainElement::ManipulateForm::CIRCULAR;
		Terrain::ManipulableTerrainElement::ManipulateType m_manipulateType = Terrain::ManipulableTerrainElement::ManipulateType::RAISE;
		float m_manipulateStrength = 1.0f;
		float m_manipulateRadius = 5.0f;

		void renderManipulationSettings();
		void updateMouseInformation();
		void checkMouseInput();
		void renderMouseInformation();
		void renderManualManipulation();
	};
}