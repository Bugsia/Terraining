#include "Terrain/TerrainManager.h"

namespace Terrain {
	TerrainManager::TerrainManager(terrain_settings terrainSettings) : settings(std::make_shared<terrain_settings>(terrainSettings)) {
		TraceLog(LOG_DEBUG, "TerrainManager: New TerrainManager created");
	}

	Model TerrainManager::newModel() {
		Model model = { 0 };
		model.transform = MatrixIdentity();

		return model;
	}

	void TerrainManager::generateNewManipulableTerrains() {
		int projectedNumElements = pow(settings->radius / (std::min(settings->numHeight, settings->numWidth) * settings->spacing) * 2, 2); // The approximate ammount of quadratic elements that would be in a rectange with a side length of the spawning circumfrence
		elements = std::unordered_set<ManipulableTerrain>(projectedNumElements);

		int counter = 0;
		bool maxElementsReached = false;
		for (int x = 0; x < round(settings->radius / ((settings->numWidth - 1) * settings->spacing)); x++) {
			for (int z = 0; z < round(getSpawnHeightAtXPos(x * ((settings->numWidth - 1) * settings->spacing), settings->radius) / ((settings->numHeight - 1) * settings->spacing)); z++) {
				// Add 4 elements, one for each quadrant
				for (int i = -1; i <= 1; i += 2) {
					for (int n = -1; n <= 1; n += 2) {
						if (elements.size() >= settings->maxNumElements) {
							maxElementsReached = true;
							break;
						}

						initialiseAndAddNewElement(elements, { x, i, z, n });
					}
					if (maxElementsReached) break;
				}
				if (maxElementsReached) break;
			}
			if (maxElementsReached) break;
		}
	}

	void TerrainManager::initialiseAndAddNewElement(std::unordered_set<ManipulableTerrain>& newElements, const PositionIdentifier& posId) {
		// Directly emplace a new ManipulableTerrain into the container
		auto result = newElements.emplace(settings, posId);
		if (result.second) { // Check if insertion was successful
			ManipulableTerrain& newElement = const_cast<ManipulableTerrain&>(*result.first);
			newElement.setModelUploaded(modelUploaded);
			newElement.initialiseMesh();
			newElement.initialiseElementWithNoiseTerrain(noiseSettings);
			newElement.Upload();

			TraceLog(LOG_DEBUG, "Terrain: New element has been created", newElement.getId());
		}

		// ManipulableTerrain newElement(settings, posId);
		// newElement.setModelUploaded(modelUploaded);
		// newElement.initialiseMesh();
		// newElement.initialiseElementWithNoiseTerrain(noiseSettings);
		// newElement.Upload();
		// 
		// TraceLog(LOG_DEBUG, "Terrain: New element has been created", newElement.getId());
		// 
		// newElements.emplace(std::move(newElement));
	}

	float TerrainManager::getSpawnHeightAtXPos(const float x, const float spawnRadius) {
		return std::max(0., sqrt(pow(spawnRadius, 2) - pow(x, 2)));
	}

	void TerrainManager::loadElementsIntoModel() {
		m_model.meshCount = elements.size();
		m_model.meshes = (Mesh*)RL_CALLOC(m_model.meshCount, sizeof(Mesh));

		int index = 0;
		for (std::unordered_set<ManipulableTerrain>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrain& element = const_cast<ManipulableTerrain&>(*it); // Const can be cast away since the hash relevant data is not changed
			m_model.meshes[index] = element.refMesh();
			index++;
		}
	}

	void TerrainManager::initializeModelMaterials() {
		// Initializing default material
		m_model.materialCount = 1;
		m_model.materials = (Material*)RL_CALLOC(m_model.materialCount, sizeof(Material));

		m_model.materials[0] = LoadMaterialDefault();

		// Giving every mesh the default material
		m_model.meshMaterial = (int*)RL_CALLOC(m_model.meshCount, sizeof(int));
		for (int i = 0; i < m_model.meshCount; i++) {
			m_model.meshMaterial[i] = 0;
		}
	}

	void TerrainManager::updateElementsNoise() {
		for (std::unordered_set<ManipulableTerrain>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrain& element = const_cast<ManipulableTerrain&>(*it); // Const can be cast away since the hash relevant data is not changed
			element.UnloadLayers();
			element.updateNoiseLayers();
			element.randomizeTerrain();
			element.updateNormals();
			element.reloadMeshData();
		}
	}

	void TerrainManager::updateModel() {
		RL_FREE(m_model.meshes);
		RL_FREE(m_model.materials);
		RL_FREE(m_model.meshMaterial);
		loadElementsIntoModel();
		initializeModelMaterials();
		updateBoundingBox();
	}

	void TerrainManager::generateDefaultTerrain() {
		generateNewManipulableTerrains();

		TraceLog(LOG_DEBUG, "Terrain: Default terrain has been generated");
	}

	void TerrainManager::initializeModel() {
		m_model = newModel();

		loadElementsIntoModel();
		initializeModelMaterials();
		updateBoundingBox();

		TraceLog(LOG_DEBUG, "Terrain: Model has been initialized");

		*modelUploaded = true;
	}

	void TerrainManager::initializeNoise() {
		noiseSettings = std::make_shared<Noise::noise_settings>(Noise::newNoiseSettings());
		Noise::getDefaultNoiseSettings(noiseSettings);

		TraceLog(LOG_DEBUG, "Terrain: Noise has been initialized");
	}

	void TerrainManager::updateTerrainNoise() {
		updateElementsNoise();

		TraceLog(LOG_DEBUG, "Terrain: Noise has been updated");
	}

	void TerrainManager::updateTerrain(float oldSpawnRadius) {
		// Check for maxNumElements
		if (elements.size() > settings->maxNumElements) {
			std::unordered_set<ManipulableTerrain>::iterator start = std::next(elements.begin(), settings->maxNumElements);
		
			// Free memory used by the elements and delete from the set
			for (std::unordered_set<ManipulableTerrain>::iterator it = start; it != elements.end();) {
				ManipulableTerrain& element = const_cast<ManipulableTerrain&>(*it); // Const can be cast away since the hash relevant data is not changed
				element.Unload();
				it = elements.erase(it);
			}
		
			updateModel();
		}
		
		// Check for radius and maxNumElements increase (for example when circle has been cutoff, so increase elements if that happened)
		if (elements.size() < settings->maxNumElements || settings->radius != oldSpawnRadius) {
			relocateElements();
		
			updateModel();
		}

		TraceLog(LOG_DEBUG, "Terrain: Terrain has been updated");
	}

	void TerrainManager::renewTerrain() {
		UnloadModel(m_model);
		*modelUploaded = false;

		for (std::unordered_set<ManipulableTerrain>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrain& element = const_cast<ManipulableTerrain&>(*it); // Const can be cast away since the hash relevant data is not changed
			element.UnloadLayers();
		}

		generateNewManipulableTerrains();
		initializeModel();

		TraceLog(LOG_DEBUG, "Terrain: Terrain has been renewed");
	}

	void TerrainManager::relocateElements() {
		TraceLog(LOG_DEBUG, "Terrain: Relocating elements of terrain");

		elements.clear();
		std::unordered_set<ManipulableTerrain> newElements;

		bool maxElementsReached = false;
		for (int x = 0; x < round(settings->radius / ((settings->numWidth - 1) * settings->spacing)); x++) {
			for (int z = 0; z < round(getSpawnHeightAtXPos(x * ((settings->numWidth - 1) * settings->spacing), settings->radius) / ((settings->numHeight - 1) * settings->spacing)); z++) {
				// Add 4 elements, one for each quadrant
				for (int i = -1; i <= 1; i += 2) {
					for (int n = -1; n <= 1; n += 2) {
						if (newElements.size() >= settings->maxNumElements) {
							maxElementsReached = true;
							break;
						}

						// DOESNT WORK FOR SOME REASONS. LEAVES HOLES IN THE TERRAIN
						// Check if there is already a terrain in this position
						// if (elements.size() != 0) {
						// 	ManipulableTerrain element({ x, i, z, n });
						// 	auto elementIt = elements.find(element);
						// 
						// 	if (elementIt != elements.end()) {
						// 		newElements.insert((*elementIt));
						// 		elements.erase(elementIt);
						// 		continue;
						// 	}
						// }

						// There is no terrain there, make new one
						initialiseAndAddNewElement(newElements, { x, i, z, n });
					}
					if (maxElementsReached) break;
				}
				if (maxElementsReached) break;
			}
			if (maxElementsReached) break;
		}

		// Set new elements
		elements = std::move(newElements);
	}

	void TerrainManager::draw() {
		// activate();
		ModelEntity::draw();
		// deactivate();
	}

	std::shared_ptr<terrain_settings> TerrainManager::refSettings() {
		return settings;
	}

	std::shared_ptr<Noise::noise_settings> TerrainManager::refNoiseSettings() {
		return noiseSettings;
	}

	RayCollision TerrainManager::getRayCollisionWithTerrain(Ray ray) {
		RayCollision hit = { 0 };

		for (std::unordered_set<ManipulableTerrain>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrain& element = const_cast<ManipulableTerrain&>(*it); // Const can be cast away since the hash relevant data is not changed
			RayCollision boundingBoxHit = GetRayCollisionBox(ray, element.getBoundingBox());
			if (boundingBoxHit.hit) {
				RayCollision elementHit = GetRayCollisionMesh(ray, element.refMesh(), m_model.transform);
				if (elementHit.hit) {
					hit = elementHit;
					break;
				}
			}
		}

		return hit;
	}

	RayCollision TerrainManager::getRayCollisionWithTerrain(Ray ray, RayCollision boundingBoxHit) {
		if (!boundingBoxHit.hit) return getRayCollisionWithTerrain(ray);
		RayCollision hit = { 0 };

		// Look from bounding box hit outwards
		// TODO: Implement

		return getRayCollisionWithTerrain(ray);
	}
}