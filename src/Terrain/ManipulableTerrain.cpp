#include "Terrain/ManipulableTerrain.h"

namespace Terrain {
	ManipulableTerrain::~ManipulableTerrain() {}

	ManipulableTerrain::ManipulableTerrain(std::shared_ptr<terrain_settings> settings, PositionIdentifier posId) : TerrainElement(settings, posId) { }

	ManipulableTerrain::ManipulableTerrain(PositionIdentifier posId) : TerrainElement(posId) { }

	ManipulableTerrain::ManipulableTerrain(const TerrainElement& other) : TerrainElement(other) { }

	void ManipulableTerrain::manipulateTerrain(ManipulateDir dir, ManipulateForm form, ManipulateType type, float strength, float radius, Vector3 relativePosition) {
		ValidIndices indices = getValidIndices(radius, relativePosition);

		// ?? Maybe simplify math(ideally making it faster) by not going through Vector2 but instead directly going of indexX and indexZ
		int index = indices.startIndex;
		for (int x = 0; x < indices.width; x++, index += settings->numHeight - indices.height) {
			for (int z = 0; z < indices.height; z++, index++) {
				if (index < 0 || index >= settings->numWidth * settings->numHeight) continue; // TODO: Improve error handling. Ideally in such a way that getValidIndices only give the onces on this terain
				float strengthFactor = manipulationStrength(form, radius, { relativePosition.x, relativePosition.z }, { x * settings->spacing, z * settings->spacing });
				manipulateVertex(dir, type, strength * strengthFactor, index);
			}
		}
	}

	ManipulableTerrain::ValidIndices ManipulableTerrain::getValidIndices(float radius, Vector3 position) {
		float width = radius * 2;
		float height = radius * 2;
		
		// move position to be inside of the terrain element
		if (position.x < 0.0f) {
			width += position.x;
			position.x = 0.0f;
		}
		else if (position.x > settings->numWidth * settings->spacing) return { 0, 0, 0 };
		if (position.z < 0.0f) {
			height += position.z;
			position.z = 0.0f;
		}
		else if (position.z > settings->numHeight * settings->spacing) return { 0, 0, 0 };
		
		// round position down to the nearest multiple of spacing
		float x = static_cast<int>(position.x / settings->spacing) * settings->spacing;
		float z = static_cast<int>(position.z / settings->spacing) * settings->spacing;

		// calculate start index of position
		int startIndex = (x / settings->spacing) * settings->numHeight + (z / settings->spacing);

		return { startIndex, static_cast<int>(width / settings->spacing), static_cast<int>(height / settings->spacing) };
	}

	float ManipulableTerrain::manipulationStrength(ManipulateForm form, float radius, Vector2 center, Vector2 position) {
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

	void ManipulableTerrain::manipulateVertex(ManipulateDir dir, ManipulateType type, float strength, int index) {
		float* vertexToManipulate;
		
		switch (dir) {
		case ManipulateDir::X:
			vertexToManipulate = &m_mesh.vertices[index];
			break;
		case ManipulateDir::Y:
			vertexToManipulate = &m_mesh.vertices[index + 1];
			break;
		case ManipulateDir::Z:
			vertexToManipulate = &m_mesh.vertices[index + 2];
			break;
		case ManipulateDir::NORMAL:
			TraceLog(LOG_WARNING, "Manipulation along normal is not yet implemented!");
			break;
		}

		switch (type) {
		case ManipulateType::RAISE:
			*vertexToManipulate += strength;
			break;
		case ManipulateType::LOWER:
			*vertexToManipulate  -= strength;
			break;
		}
	}
}