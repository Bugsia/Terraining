#pragma once
#include <raylib.h>
#include <memory>
#include <unordered_set>
#include "Terrain/ManipulableTerrain.h"
#include "ModelEntity.h"
#include "ShaderHandler.h"

// Custom hash function for ManipulableTerrain
namespace std {
	template<>
	struct hash<Terrain::ManipulableTerrain> {
		size_t operator()(const Terrain::ManipulableTerrain& element) const {
			return std::hash<unsigned int>()(element.getId());
		}
	};
}

namespace Terrain {
	class TerrainManager : public ModelEntity<Vector3>, public ShaderHandler {
	public:
		TerrainManager(terrain_settings terrainSettings);

		void generateDefaultTerrain();
		void initializeModel();
		void initializeNoise();
		void updateTerrain(float oldRadius);
		void renewTerrain();
		void relocateElements();
		void updateTerrainNoise();
		void manipulateTerrain(ManipulableTerrain::ManipulateDir dir, ManipulableTerrain::ManipulateForm form, ManipulableTerrain::ManipulateType type, float strength, float radius, Vector3 position);
		void draw();

		// GETTER AND SETTER
		std::shared_ptr<terrain_settings> refSettings();
		std::shared_ptr<Noise::noise_settings> refNoiseSettings();
		RayCollision getRayCollisionWithTerrain(Ray ray);
		RayCollision getRayCollisionWithTerrain(Ray ray, RayCollision boundingBoxHit);

	protected:
		std::shared_ptr<terrain_settings> settings; // The terrain settings
		std::shared_ptr<Noise::noise_settings> noiseSettings; // The noise settings

		std::shared_ptr<bool> modelUploaded = std::make_shared<bool>(false); // True if the model has been uploaded to the GPU, false otherwise
		std::unordered_set<ManipulableTerrain> elements; // The terrain elements

		Model newModel();
		void generateNewManipulableTerrains();
		void initialiseAndAddNewElement(std::unordered_set<ManipulableTerrain>& newElements, const PositionIdentifier& posId);
		float getSpawnHeightAtXPos(const float x, const float spawnRadius);
		void loadElementsIntoModel(); // Sets meshCount of model and loads the meshes of the elements into the model
		void initializeModelMaterials(); // Initializes the model with the default material and sets it to be the material of every mesh
		void updateElementsNoise();
		void updateModel();
	};
}