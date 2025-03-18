#include "Terrain/TerrainManager.h"

namespace Terrain {
	TerrainManager::TerrainManager(std::string name, terrain_settings terrainSettings, Noise::noise_settings noiseSettings) : TemplateTerrainManager(name, terrainSettings, noiseSettings) {
	}

	TerrainManager::TerrainManager(std::string name, const FileAdapter& settings) : TemplateTerrainManager(name, settings) {
	}

	void TerrainManager::initialiseAndAddNewElement(std::unordered_set<TerrainElement>& newElements, const PositionIdentifier& posId) {
		std::pair<std::unordered_set<TerrainElement>::iterator, bool> result = newElements.emplace(&m_terrainSettings, posId);
		if (result.second) { // Check if insertion was successful
			TerrainElement* newElement = const_cast<TerrainElement*>(&*result.first);
			newElement->setModelUploaded(&modelUploaded);
			newElement->initialiseMesh();
			newElement->initialiseElementWithNoiseTerrain(&m_noiseSettings);
			newElement->getUploadFlag()->store(true);

			TraceLog(LOG_DEBUG, "TerrainManager: New element has been created", newElement->getId());
		}
		else {
			TraceLog(LOG_WARNING, "TerrainManager: New Element could not be emplaced into elements vector. Either it exists already or an error occured on construction");
		}
	}

	/**
	* It removes and then recalculates the noise and height manipulations of the given element.
	*/
	void TerrainManager::reloadElement(TerrainElement* element) {
		element->UnloadLayers();
		element->updateNoiseLayers();
		element->randomizeTerrain();
		element->updateNormals();
		element->getReloadFlag()->store(true);
	}
}