#include "DebugGui/ManipulableTerrainDebugGui.h"

namespace DebugGui {
	ManipulableTerrainDebugGui::ManipulableTerrainDebugGui(std::string name, Terrain::TerrainManager& terrain, Camera& camera) : Gui(name), m_terrain(terrain), m_camera(camera) {}

	bool ManipulableTerrainDebugGui::render() {
		ImGui::Begin(m_name.c_str(), &m_open);
		defaultStyle();

		if (m_checkMouse) updateMouseInformation();
		renderMouseInformation();
		
		renderManualManipulation();

		ImGui::End();

		return m_open;
	}

	void ManipulableTerrainDebugGui::renderMouseInformation() {
		ImGui::SeparatorText("Mouse Hit Information");
		ImGui::Checkbox("Check Mouse", &m_checkMouse);
		Vector2 mousePos = GetMousePosition();
		ImGui::Text("Mouse Position: (%f, %f)", mousePos.x, mousePos.y);
		ImGui::Text("Collision hit: %s", m_mouseCollision.hit ? "true" : "false");
		ImGui::Text("Collision distance: %f", m_mouseCollision.distance);
		ImGui::Text("Collision position: (%f, %f, %f)", m_mouseCollision.point.x, m_mouseCollision.point.y, m_mouseCollision.point.z);
		ImGui::Text("Collision normal: (%f, %f, %f)", m_mouseCollision.normal.x, m_mouseCollision.normal.y, m_mouseCollision.normal.z);
	}

	void ManipulableTerrainDebugGui::updateMouseInformation() {
		m_mouseRay = GetMouseRay(GetMousePosition(), m_camera);
		RayCollision collision = GetRayCollisionBox(m_mouseRay, m_terrain.getBoundingBox());
		if (collision.hit) {
			RayCollision terrainCollsion = m_terrain.getRayCollisionWithTerrain(m_mouseRay, collision);
			if (terrainCollsion.hit) {
				m_mouseCollision = terrainCollsion;
			}
		}
	}

	void ManipulableTerrainDebugGui::renderManualManipulation() {
		ImGui::SeparatorText("Mouse Hit Information");
		
	}
}