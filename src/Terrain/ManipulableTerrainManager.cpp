#include "Terrain/ManipulableTerrainManager.h"

namespace Terrain {
	ManipulableTerrainManager::ManipulableTerrainManager(std::string name, terrain_settings terrainSettings, Noise::noise_settings noiseSettings) : TemplateTerrainManager(name, terrainSettings, noiseSettings) {
	}

	ManipulableTerrainManager::ManipulableTerrainManager(std::string name, const FileAdapter& settings) : TemplateTerrainManager(name, settings) {
		if (loadManipulations(settings)) {
			for (const ManipulableTerrainElement& constElement : m_elements) {
				ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(constElement);
				element.addDifference();
			}
		}
	}

	/**
	* @params position In global space
	*/
	void ManipulableTerrainManager::manipulateTerrain(ManipulableTerrainElement::ManipulateDir dir, ManipulableTerrainElement::ManipulateForm form, ManipulableTerrainElement::ManipulateType type, float strength, float radius, Vector3 position) {
		std::vector<PositionIdentifier> posIds = getPositionIdentifiersInRadius(position, radius + std::max(m_terrainSettings.numWidth, m_terrainSettings.numHeight) * m_terrainSettings.spacing);

		for (PositionIdentifier posId : posIds) {
			std::unordered_set<ManipulableTerrainElement>::iterator it = m_elements.find(ManipulableTerrainElement(posId));
			if (it == m_elements.end()) continue;
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it);
			element.manipulateTerrain(dir, form, type, strength, radius, Vector3Subtract(position, element.getPosition()));
		}
	}

	void ManipulableTerrainManager::clearDifference() {
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = m_elements.begin(); it != m_elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it);
			element.clearDifference();
		}
	}

	void ManipulableTerrainManager::initialiseAndAddNewElement(std::unordered_set<ManipulableTerrainElement>& newElements, const PositionIdentifier& posId) {
		TraceLog(LOG_INFO, "MANIPULABLE GETS CALLED!!!! HAHAHA");
		float* newDiff = nullptr;
		std::unordered_map<PositionIdentifier, float*>::iterator it = m_manipulations.find(posId);
		if (it == m_manipulations.end()) { // It doesnt exist yet, so make a new one
			newDiff = new float[m_terrainSettings.numWidth * m_terrainSettings.numHeight * 3];
			m_manipulations[posId] = newDiff;
		}
		else if (std::isnan(*it->second)) { // The previous manipulation was empty, so reuse it
			*(it->second) = 0.0f;
			newDiff = it->second;
		}

		auto result = newElements.emplace(&m_terrainSettings, posId, newDiff);
		if (result.second) { // Check if insertion was successful
			ManipulableTerrainElement* newElement = const_cast<ManipulableTerrainElement*>(&*result.first);
			newElement->setModelUploaded(&modelUploaded);
			newElement->initialiseMesh();
			newElement->initialiseElementWithNoiseTerrain(&m_noiseSettings);
			if (!newDiff) newElement->loadDifference(m_manipulations[posId]);
			newElement->getUploadFlag()->store(true);

			TraceLog(LOG_DEBUG, "TerrainManager: New element has been created", newElement->getId());
		}
		else {
			TraceLog(LOG_WARNING, "TerrainManager: New Element could not be emplaced into elements vector. Either it exists already or an error occured on construction");
		}
	}

	bool ManipulableTerrainManager::loadManipulations(const FileAdapter& settings) {
		const FileAdapter& manipulations = settings.getSubElement(std::string(JsonKeys::manipulations));
		if (manipulations.getKey() == "") {
			TraceLog(LOG_WARNING, "TerrainManager: Manipulations not found in file %s", settings.getFilename().c_str());
			return false;
		}

		try {
			for (std::string key : manipulations.getAllSubKeys()) {
				const FileAdapter& manipulation = manipulations.getSubElement(key);
				std::vector<std::any> manipulationsVector = manipulation.getArray(std::string(JsonKeys::manipulationsArray)).getValue();
				float* differences = new float[manipulationsVector.size()];
				for (int i = 0; i < manipulationsVector.size(); i++) {
					differences[i] = std::any_cast<float>(manipulationsVector[i]);
				}
				m_manipulations[getPositionIdentifierFromKey(key)] = differences;
			}
		}
		catch (std::bad_any_cast& e) {
			TraceLog(LOG_WARNING, "TerrainManager: bad_any_cast in loadManipulations: %s", e.what());
			m_manipulations.clear();
			return false;
		}

		TraceLog(LOG_DEBUG, "TerrainManager: Manipulations have been loaded from file");
		return true;
	}

	void ManipulableTerrainManager::saveManipulations(FileAdapter& file) const {
		FileAdapter& manipulations = file.getSubElement(std::string(JsonKeys::manipulations));
		manipulations.clear();

		for (auto& [posId, value] : m_manipulations) {
			if (std::isnan(*value)) continue; // No difference marked with NaN because Element doesnt exist anymore
			bool diffFound = false;
			for (int i = 0; i < m_terrainSettings.numWidth * m_terrainSettings.numHeight * 3; i++) {
				if (value[i] != 0.0f) {
					diffFound = true;
					break;
				}
			}
			if (!diffFound) continue;

			std::string key = getKeyFromPositionIdentifier(PositionIdentifier(1,2,3,4));
			FileAdapter& curManipulation = manipulations.getSubElement(key);
			curManipulation.clear();
			std::vector<std::any> manipulationsVector(value, value + (m_terrainSettings.numWidth * m_terrainSettings.numHeight * 3));
			curManipulation.addArray(FileAdapter::FileArray(std::string(JsonKeys::manipulationsArray), FileAdapter::ValueType::FLOAT, manipulationsVector));
		}

		TraceLog(LOG_DEBUG, "TerrainManager: Manipulations have been saved to file");
	}

	void ManipulableTerrainManager::reloadElement(ManipulableTerrainElement* element) {
		element->UnloadLayers();
		element->updateNoiseLayers();
		element->randomizeTerrain();
		element->updateNormals();
		element->addDifference();
		element->getReloadFlag()->store(true);
	}

	std::string ManipulableTerrainManager::getKeyFromPositionIdentifier(PositionIdentifier posId) const {
		return "x" + std::to_string(posId.x) + "i" + std::to_string(posId.i) + "z" + std::to_string(posId.z) + "n" + std::to_string(posId.n);
	}

	PositionIdentifier ManipulableTerrainManager::getPositionIdentifierFromKey(std::string key) const {
		PositionIdentifier posId;
		posId.x = std::stoi(key.substr(1, key.find("i") - 1));
		posId.i = std::stoi(key.substr(key.find("i") + 1, key.find("z") - key.find("i") - 1));
		posId.z = std::stoi(key.substr(key.find("z") + 1, key.find("n") - key.find("z") - 1));
		posId.n = std::stoi(key.substr(key.find("n") + 1, key.length() - key.find("n") - 1));
		return posId;
	}
}