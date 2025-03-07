#include "Terrain/ManipulableTerrainElement.h"

namespace Terrain {
	ManipulableTerrainElement::~ManipulableTerrainElement() {}

	ManipulableTerrainElement::ManipulableTerrainElement(std::shared_ptr<terrain_settings> settings, PositionIdentifier posId) : TerrainElement(settings, posId) { 
		initialiseDifference();
	}

	ManipulableTerrainElement::ManipulableTerrainElement(PositionIdentifier posId) : TerrainElement(posId) { }

	ManipulableTerrainElement::ManipulableTerrainElement(const TerrainElement& other) : TerrainElement(other) {
		initialiseDifference();
	}

	ManipulableTerrainElement::ManipulableTerrainElement(const ManipulableTerrainElement& other) : TerrainElement(other) {
		m_difference = new float[settings->numWidth * settings->numHeight * 3];
		for (int i = 0; i < settings->numWidth * settings->numHeight * 3; i++) {
			m_mesh.vertices[i] += other.m_difference[i];
			m_difference[i] = other.m_difference[i];
		}
	}

	void ManipulableTerrainElement::initialiseDifference() {
		m_difference = new float[settings->numWidth * settings->numHeight * 3];
		for (int i = 0; i < settings->numWidth * settings->numHeight * 3; i++) {
			m_difference[i] = 0.0f;
		}
	}

	void ManipulableTerrainElement::loadDifference(std::vector<std::any> heightVector) {
		if (heightVector.size() > settings->numHeight * settings->numHeight * 3) return;
		if (m_difference == nullptr) initialiseDifference();
		
		for (int i = 0; i < heightVector.size(); i++) {
			m_difference[i] = std::any_cast<float>(heightVector[i]);
			m_mesh.vertices[i] += m_difference[i];
		}

		reloadMeshData();
		m_hasDifference = true;
	}

	void ManipulableTerrainElement::manipulateTerrain(ManipulateDir dir, ManipulateForm form, ManipulateType type, float strength, float radius, Vector3 relativePosition) {
		ValidIndices indices = getValidIndices(radius, relativePosition);
		if (indices.startIndex == -1) return;

		// TraceLog(LOG_INFO, "start index: %i, width: %i, height: %i", indices.startIndex, indices.width, indices.height);
		Vector2 vertexPos = { m_mesh.vertices[indices.startIndex * 3] - m_position.x, m_mesh.vertices[indices.startIndex * 3 + 2] - m_position.z };
		int index = indices.startIndex * 3;
		for (int x = 0; x < indices.width; x++, index += (settings->numHeight - indices.height) * 3, vertexPos.x += settings->spacing, vertexPos.y = m_mesh.vertices[indices.startIndex * 3 + 2] - m_position.z) {
			for (int z = 0; z < indices.height; z++, index += 3, vertexPos.y += settings->spacing) {
				float strengthFactor = manipulationStrength(form, radius, { relativePosition.x, relativePosition.z }, vertexPos);
				manipulateVertex(dir, type, strength * strengthFactor, index);
			}
		}

		reloadMeshData();
		m_hasDifference = true;
	}

	void ManipulableTerrainElement::removeDifference() {
		for (int i = 0; i < settings->numWidth * settings->numHeight * 3; i++) {
			m_mesh.vertices[i] -= m_difference[i];
		}

		reloadMeshData();
	}

	void ManipulableTerrainElement::addDifference() {
		for (int i = 0; i < settings->numWidth * settings->numHeight * 3; i++) {
			m_mesh.vertices[i] += m_difference[i];
		}

		reloadMeshData();
	}

	ManipulableTerrainElement::ValidIndices ManipulableTerrainElement::getValidIndices(float radius, Vector3 position) {
		// round position down to the nearest multiple of spacing
		float x = static_cast<int>((position.x - radius) / settings->spacing) * settings->spacing;
		float z = static_cast<int>((position.z - radius) / settings->spacing) * settings->spacing;

		float width = radius * 2;
		float height = radius * 2;
		
		// move position to be inside of the terrain element
		// check if whole area is outside of the terrain element
		if (x + width < 0.0f) return { -1, 0, 0 };
		else if (x > settings->numWidth * settings->spacing) return { -1, 0, 0 };
		if (z + height < 0.0f) return { -1, 0, 0 };
		else if (z > settings->numHeight * settings->spacing) return { -1, 0, 0 };

		// check and fix partial areas being outside
		if (x < 0.0f) {
			width += x;
			x = 0.0f;
		}
		if (x + width > settings->numWidth * settings->spacing) width = settings->numWidth * settings->spacing - x;
		if (z < 0.0f) {
			height += z;
			z = 0.0f;
		}
		if (z + height > settings->numHeight * settings->spacing) height = settings->numHeight * settings->spacing - z;

		// calculate start index of position
		int startIndex = (x / settings->spacing) * settings->numHeight + (z / settings->spacing);

		return { startIndex, static_cast<int>(width / settings->spacing), static_cast<int>(height / settings->spacing) };
	}

	float ManipulableTerrainElement::manipulationStrength(ManipulateForm form, float radius, Vector2 center, Vector2 position) {
		float strengthFactor = 0.0f;
		float distance;

		switch (form) {
		case ManipulateForm::CIRCULAR:
			distance = Vector2Distance(center, position);
			if (distance <= radius) {
				strengthFactor = 1.0f - (distance / radius);
			}
			break;
		case ManipulateForm::SQUARE:
			strengthFactor = 1.0f;
			break;
		}

		return strengthFactor;
	}

	void ManipulableTerrainElement::manipulateVertex(ManipulateDir dir, ManipulateType type, float strength, int index) {
		int manipulationIndex;
		
		switch (dir) {
		case ManipulateDir::X:
			manipulationIndex = index;
			break;
		case ManipulateDir::Y:
			manipulationIndex = index + 1;
			break;
		case ManipulateDir::Z:
			manipulationIndex = index + 2;
			break;
		case ManipulateDir::NORMAL:
			TraceLog(LOG_WARNING, "Manipulation along normal is not yet implemented!");
			return;
			break;
		default:
			TraceLog(LOG_WARNING, "Invalid ManipulateDir!");
			return;
		}

		switch (type) {
		case ManipulateType::RAISE:
			m_mesh.vertices[manipulationIndex] += strength;
			m_difference[manipulationIndex] += strength;
			break;
		case ManipulateType::LOWER:
			m_mesh.vertices[manipulationIndex] -= strength;	
			m_difference[manipulationIndex] -= strength;
			break;
		}
	}

	const float* ManipulableTerrainElement::getDifference() const {
		return m_difference;
	}

	bool ManipulableTerrainElement::getHasDifference() const {
		return m_hasDifference;
	}
}