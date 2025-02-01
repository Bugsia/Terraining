#include "DebugGui/ManipulableTerrainDebugGui.h"

namespace DebugGui {
	ManipulableTerrainDebugGui::ManipulableTerrainDebugGui(std::string name, Terrain::TerrainManager& terrain, Camera& camera) : Gui(name), m_terrain(terrain), m_camera(camera) {}

	bool ManipulableTerrainDebugGui::render() {
		ImGui::Begin(m_name.c_str(), &m_open);
		defaultStyle();

		renderManipulationSettings();

		if (m_checkMouse) updateMouseInformation();
		checkMouseInput();
		renderMouseInformation();
		
		renderManualManipulation();

		ImGui::End();

		return m_open;
	}

	void ManipulableTerrainDebugGui::renderManipulationSettings() {
		ImGui::SeparatorText("Manipulation Settings");
		ImGui::Text("Manipulation Direction");
		ImGui::SameLine();
		ImGui::RadioButton("X", (int*)&m_manipulateDir, Terrain::ManipulableTerrain::ManipulateDir::X);
		ImGui::SameLine();
		ImGui::RadioButton("Y", (int*)&m_manipulateDir, Terrain::ManipulableTerrain::ManipulateDir::Y);
		ImGui::SameLine();
		ImGui::RadioButton("Z", (int*)&m_manipulateDir, Terrain::ManipulableTerrain::ManipulateDir::Z);
		
		ImGui::Text("Manipulation Form");
		ImGui::SameLine();
		ImGui::RadioButton("Circular", (int*)&m_manipulateForm, Terrain::ManipulableTerrain::ManipulateForm::CIRCULAR);
		ImGui::SameLine();
		ImGui::RadioButton("Square", (int*)&m_manipulateForm, Terrain::ManipulableTerrain::ManipulateForm::SQUARE);
	
		ImGui::Text("Manipulation Type");
		ImGui::SameLine();
		ImGui::RadioButton("Raise", (int*)&m_manipulateType, Terrain::ManipulableTerrain::ManipulateType::RAISE);
		ImGui::SameLine();
		ImGui::RadioButton("Lower", (int*)&m_manipulateType, Terrain::ManipulableTerrain::ManipulateType::LOWER);

		ImGui::SliderFloat("Strength", &m_manipulateStrength, 0.0f, 25.0f);
		ImGui::SliderFloat("Radius", &m_manipulateRadius, 0.0f, 25.0f);
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

	void ManipulableTerrainDebugGui::checkMouseInput() {
		if (m_mouseCollision.hit && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			m_terrain.manipulateTerrain(m_manipulateDir, m_manipulateForm, m_manipulateType, m_manipulateStrength, m_manipulateRadius, m_manipulationPosition);
			m_checkMouse = false; // Keep manipulation position fixed until mouse is released
		}
		else if (m_mouseCollision.hit && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
			m_checkMouse = true;
		}
	}

	void ManipulableTerrainDebugGui::updateMouseInformation() {
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
			m_mouseCollision.hit = false;
			return;
		}
		m_mouseRay = GetMouseRay(GetMousePosition(), m_camera);
		RayCollision collision = GetRayCollisionBox(m_mouseRay, m_terrain.getBoundingBox());
		if (collision.hit) {
			RayCollision terrainCollsion = m_terrain.getRayCollisionWithTerrain(m_mouseRay, collision);
			if (terrainCollsion.hit) {
				m_mouseCollision = terrainCollsion;
				m_manipulationPosition = m_mouseCollision.point;
			}
			m_mouseCollision.hit = terrainCollsion.hit; // If no hit has been made update this in the gui but retain other data of last succesful hit
		}
	}

	void ManipulableTerrainDebugGui::renderManualManipulation() {
		ImGui::SeparatorText("Manual Manipulation");
		ImGui::Text("Manipulation Position");
		ImGui::SameLine();
		ImGui::SliderFloat3("", (float*)&m_manipulationPosition, -100.0f, 100.0f);
	
		ImGui::Button("Manipulate");
		if (ImGui::IsItemActive()) m_terrain.manipulateTerrain(m_manipulateDir, m_manipulateForm, m_manipulateType, m_manipulateStrength, m_manipulateRadius, m_manipulationPosition);
	}
}