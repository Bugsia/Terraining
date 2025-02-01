#include "Terrain/ManipulableTerrain.h"

namespace Terrain {
	ManipulableTerrain::~ManipulableTerrain() {}

	ManipulableTerrain::ManipulableTerrain(std::shared_ptr<terrain_settings> settings, PositionIdentifier posId) : TerrainElement(settings, posId) { }

	ManipulableTerrain::ManipulableTerrain(PositionIdentifier posId) : TerrainElement(posId) { }

	ManipulableTerrain::ManipulableTerrain(const TerrainElement& other) : TerrainElement(other) { }

	void ManipulableTerrain::manipulateTerrain(ManipulateDir dir, ManipulateForm form, ManipulateType type, float strength, float radius, Vector3 relativePosition) {
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
	}

	ManipulableTerrain::ValidIndices ManipulableTerrain::getValidIndices(float radius, Vector3 position) {
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

		TraceLog(LOG_INFO, "X: %f, Z: %f, Width: %f, Height: %f", x, z, width, height);
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
		default:
			TraceLog(LOG_WARNING, "Invalid ManipulateDir!");
			return;
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