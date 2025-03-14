#pragma once
#include <raylib.h>
#include <memory>
#include <unordered_set>
#include <string>
#include <mutex>
#include "Terrain/ManipulableTerrainElement.h"
#include "ModelObject.h"
#include "Actor.h"
#include "FileAdapters/JSONAdapter.h"
#include "ThreadPool.h"

// Custom hash function for ManipulableTerrain
namespace std {
	template<>
	struct hash<Terrain::ManipulableTerrainElement> {
		size_t operator()(const Terrain::ManipulableTerrainElement& element) const {
			return std::hash<unsigned int>()(element.getId());
		}
	};
}

namespace Terrain {
	class TerrainManager : public ModelObject, public Actor<Vector3>, public Drawable {
	public:
		TerrainManager(std::string name, terrain_settings terrainSettings);
		TerrainManager(std::string name, std::string filename);
		TerrainManager(const FileAdapter& settings);

		void generateDefaultTerrain();
		void loadTerrainElements(const FileAdapter& elements);
		void initializeModel();
		void loadNoiseSettings(const FileAdapter& settingsFile);
		void initializeNoise();
		void updateTerrain(float oldRadius);
		void renewTerrain();
		
		void updateTerrainNoise();
		void manipulateTerrain(ManipulableTerrainElement::ManipulateDir dir, ManipulableTerrainElement::ManipulateForm form, ManipulableTerrainElement::ManipulateType type, float strength, float radius, Vector3 position);
		void update(int targetFPS);
		void draw();
		void updateElementPositions();
		void removeDifference();
		void addDifference();
		void clearDifference();

		void setThreadPool(ThreadPool* threadPool);
		void setCamera(Character* camera);

		void save() const;
		void save(std::string filename) const;
		void save(FileAdapter& file) const;

		void load(const FileAdapter& file);

		// GETTER AND SETTER
		std::shared_ptr<terrain_settings> refSettings();
		std::shared_ptr<Noise::noise_settings> refNoiseSettings();
		RayCollision getRayCollisionWithTerrain(Ray ray);
		RayCollision getRayCollisionWithTerrain(Ray ray, RayCollision boundingBoxHit);

	protected:
		std::shared_ptr<terrain_settings> settings; // The terrain settings
		std::shared_ptr<Noise::noise_settings> noiseSettings; // The noise settings

		std::shared_ptr<bool> modelUploaded = std::make_shared<bool>(false); // True if the model has been uploaded to the GPU, false otherwise
		std::unordered_set<ManipulableTerrainElement> elements; // The terrain elements
		std::atomic<bool> m_updateModel{ false };
		std::mutex m_updating; // Any thread that could cause update() to crash (example: deleting elements from elements) locks this firts preventing updating
		Vector3 center = { 0.0f, 0.0f, 0.0f };
		std::unordered_map<PositionIdentifier, std::shared_ptr<float[]>, PositionIdentifierHash> m_loadedManipulations;

		Model newModel();
		void initialiseAndAddNewElement(std::unordered_set<ManipulableTerrainElement>& newElements, const PositionIdentifier& posId);
		float getSpawnHeightAtXPos(const float x, const float spawnRadius);
		void loadElementsIntoModel(); // Sets meshCount of model and loads the meshes of the elements into the model
		void initializeModelMaterials(); // Initializes the model with the default material and sets it to be the material of every mesh
		void updateElementsNoise();
		void updateModel();
		void relocateElements();
		PositionIdentifier getPositionIdentifierFromKey(std::string key);

		void saveTerrainSettings(FileAdapter& json) const;
		void saveNoiseSettings(FileAdapter& jsone) const;
		void saveTerrainElements(FileAdapter& json) const;
	};
}