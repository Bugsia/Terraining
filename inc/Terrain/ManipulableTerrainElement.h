#pragma once
#include <raylib.h>
#include <any>
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
			LOWER,
			FLATTEN
		};

		~ManipulableTerrainElement();
		ManipulableTerrainElement(std::shared_ptr<terrain_settings> settings, PositionIdentifier posId, std::shared_ptr<float[]> heightDifference);
		ManipulableTerrainElement(PositionIdentifier posId);
		ManipulableTerrainElement(const TerrainElement& other);
		ManipulableTerrainElement(const ManipulableTerrainElement& other);

		void manipulateTerrain(ManipulateDir dir, ManipulateForm form, ManipulateType type, float strength, float radius, Vector3 relativePosition);
		void loadDifference(std::shared_ptr<float[]> heightDifference);
		void removeDifference();
		void addDifference();
		void clearDifference();

		const float* getDifference() const;
		bool getHasDifference() const;

	private:
		struct ValidIndices {
			int startIndex;
			int width;
			int height;
		};

		std::shared_ptr<float[]> m_difference = nullptr; // ManipulableTerrainElement is not the onwer of this memory. It is a pointer to the memory owned by the TerrainManager, who has it stored in a map
		bool m_hasDifference = false;

		ValidIndices getValidIndices(float radius, Vector3 position);
		float manipulationStrength(ManipulateForm form, float radius, Vector2 center, Vector2 position);
		void manipulateVertex(ManipulateDir dir, ManipulateType type, float strengthFactor, float strength, int index);
		void initialiseDifference();
	};
}