#pragma once
#include <raylib.h>
#include "TerrainElement.h"

namespace Terrain {
	class ManipulableTerrainElement : public TerrainElement {
	public:
		enum ManipulateDir {
			X,
			Y,
			Z,
			NORMAL
		};

		enum ManipulateForm {
			CIRCULAR,
			SQUARE
		};

		enum ManipulateType {
			RAISE,
			LOWER
		};

		~ManipulableTerrainElement();
		ManipulableTerrainElement(std::shared_ptr<terrain_settings> settings, PositionIdentifier posId);
		ManipulableTerrainElement(PositionIdentifier posId);
		ManipulableTerrainElement(const TerrainElement& other);
		ManipulableTerrainElement(const ManipulableTerrainElement& other);

		void manipulateTerrain(ManipulateDir dir, ManipulateForm form, ManipulateType type, float strength, float radius, Vector3 relativePosition);
		void removeDifference();
		void addDifference();

	private:
		struct ValidIndices {
			int startIndex;
			int width;
			int height;
		};

		float* m_difference;

		ValidIndices getValidIndices(float radius, Vector3 position);
		float manipulationStrength(ManipulateForm form, float radius, Vector2 center, Vector2 position);
		void manipulateVertex(ManipulateDir dir, ManipulateType type, float strength, int index);
		void initialiseDifference();
	};
}