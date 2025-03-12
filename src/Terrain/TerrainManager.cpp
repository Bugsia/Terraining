#include "Terrain/TerrainManager.h"

namespace Terrain {
	TerrainManager::TerrainManager(std::string name, terrain_settings terrainSettings) : Actor<Vector3>(name), settings(std::make_shared<terrain_settings>(terrainSettings)) {
		TraceLog(LOG_DEBUG, "TerrainManager: New TerrainManager created");
	}

	TerrainManager::TerrainManager(std::string name, std::string filename) /* : TerrainManager(JSONAdapter(filename, 4).getSubElement(name)) */ : Actor<Vector3>("LOLZ") {
		TraceLog(LOG_DEBUG, "TerrainManager: New TerrainManager created");
	}

	TerrainManager::TerrainManager(const FileAdapter& settings, Character* camera) : Actor<Vector3>(settings.getKey()) {
		m_filename = settings.getFilename();
		const FileAdapter& terrainSettingsFile = settings.getSubElement("terrain_settings");
		this->settings = std::make_shared<terrain_settings>();
		this->settings->radius = std::any_cast<float>(terrainSettingsFile.getField("radius").getValue());
		this->settings->numWidth = std::any_cast<int>(terrainSettingsFile.getField("num_width").getValue());
		this->settings->numHeight = std::any_cast<int>(terrainSettingsFile.getField("num_height").getValue());
		this->settings->maxNumElements = std::any_cast<int>(terrainSettingsFile.getField("max_num_elements").getValue());
		this->settings->spacing = std::any_cast<float>(terrainSettingsFile.getField("spacing").getValue());
		this->settings->updateWithThreadPool = std::any_cast<bool>(terrainSettingsFile.getField("update_with_thread_pool").getValue());
		this->settings->camera = camera;
		loadNoiseSettings(settings.getSubElement("noise_settings"));
		loadTerrainElements(settings.getSubElement("terrain_elements"));
		Actor::load(settings);
		TraceLog(LOG_DEBUG, "TerrainManager: New TerrainManager created");
	}

	Model TerrainManager::newModel() {
		Model model = { 0 };
		model.transform = MatrixIdentity();

		return model;
	}

	void TerrainManager::generateNewManipulableTerrains() {
		int projectedNumElements = pow(settings->radius / (std::min(settings->numHeight, settings->numWidth) * settings->spacing) * 2, 2); // The approximate ammount of quadratic elements that would be in a rectange with a side length of the spawning circumfrence
		elements = std::unordered_set<ManipulableTerrainElement>(projectedNumElements);

		bool maxElementsReached = false;
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		if (settings->camera) position = Vector3Subtract(settings->camera->getPosition(), m_position);

		// Spawning elements from the bottom left corner
		float width = (settings->numWidth - 1) * settings->spacing;
		float height = (settings->numHeight - 1) * settings->spacing;
		int numPerQuadrantX = round(settings->radius / width);
		int numPerQuadrantZ = round(settings->radius / height);
		float x = position.x - numPerQuadrantX * width;
		float z = position.z - numPerQuadrantZ * height;
		for (int i = -numPerQuadrantX; i < numPerQuadrantX; i++, x += width) {
			float circleHeight = getSpawnHeightAtXPos(x - position.x + (width / 2), settings->radius);
			for (int j = -numPerQuadrantZ; j < numPerQuadrantZ; j++, z += height) {
				if (std::abs(z - position.z + (height / 2)) > circleHeight) continue;

				int i = x < 0 ? -1 : 1;
				int posX = (x - width * std::min(0, i)) / (width * i);
				int n = z < 0 ? -1 : 1;
				int posZ = (z - height * std::min(0, n)) / (height * n);

				initialiseAndAddNewElement(elements, { posX, i, posZ, n });
				if (elements.size() >= settings->maxNumElements) {
					maxElementsReached = true;
					break;
				}
			}
			z = position.z - numPerQuadrantZ * height;
			if (maxElementsReached) break;
		}
	}

	void TerrainManager::removeDifference() {
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
			element.removeDifference();
		}
	}

	void TerrainManager::addDifference() {
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
			element.addDifference();
		}
	}

	void TerrainManager::clearDifference() {
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
			element.clearDifference();
		}
	}

	void TerrainManager::setThreadPool(ThreadPool* threadPool) {
		settings->threadPool = threadPool;
	}

	void TerrainManager::setCamera(Character* camera) {
		settings->camera = camera;
	}

	void TerrainManager::save() const {
		save(m_filename);
	}

	void TerrainManager::save(std::string filename) const {
		JSONAdapter json(filename, 4);
		save(json.getSubElement(m_name));
		json.save();
	}

	void TerrainManager::save(FileAdapter& file) const {
		Actor::save(file);
		saveTerrainSettings(file);
		saveNoiseSettings(file);
		saveTerrainElements(file);
	}

	void TerrainManager::saveTerrainSettings(FileAdapter& json) const {
		FileAdapter& settings = json.getSubElement("terrain_settings");
		settings.clear();
		settings.addField(FileAdapter::FileField("radius", FileAdapter::ValueType::FLOAT, this->settings->radius));
		settings.addField(FileAdapter::FileField("num_width", FileAdapter::ValueType::INT, this->settings->numWidth));
		settings.addField(FileAdapter::FileField("num_height", FileAdapter::ValueType::INT, this->settings->numHeight));
		settings.addField(FileAdapter::FileField("max_num_elements", FileAdapter::ValueType::INT, static_cast<int>(this->settings->maxNumElements)));
		settings.addField(FileAdapter::FileField("spacing", FileAdapter::ValueType::FLOAT, this->settings->spacing));
		settings.addField(FileAdapter::FileField("update_with_thread_pool", FileAdapter::ValueType::BOOL, this->settings->updateWithThreadPool));
	}

	void TerrainManager::saveNoiseSettings(FileAdapter& json) const {
		FileAdapter& noise = json.getSubElement("noise_settings");
		noise.clear();
		noise.addField(FileAdapter::FileField("seed", FileAdapter::ValueType::INT, noiseSettings->seed));
		int index = 0;
		for (Noise::noise_layer_settings& curNoiseLayer : noiseSettings->noiseLayerSettings) {
			FileAdapter& curNoiseLayerFile = noise.getSubElement(std::to_string(index));
			curNoiseLayerFile.clear();
			curNoiseLayerFile.addField(FileAdapter::FileField("horizontal_scale", FileAdapter::ValueType::FLOAT, curNoiseLayer.horizontalScale));
			curNoiseLayerFile.addField(FileAdapter::FileField("vertical_scale", FileAdapter::ValueType::FLOAT, curNoiseLayer.verticalScale));
			curNoiseLayerFile.addField(FileAdapter::FileField("offset_x", FileAdapter::ValueType::INT, curNoiseLayer.offsetX));
			curNoiseLayerFile.addField(FileAdapter::FileField("offset_z", FileAdapter::ValueType::INT, curNoiseLayer.offsetZ));
			curNoiseLayerFile.addField(FileAdapter::FileField("lacunarity", FileAdapter::ValueType::FLOAT, curNoiseLayer.lacunarity));
			curNoiseLayerFile.addField(FileAdapter::FileField("gain", FileAdapter::ValueType::FLOAT, curNoiseLayer.gain));
			curNoiseLayerFile.addField(FileAdapter::FileField("octaves", FileAdapter::ValueType::INT, curNoiseLayer.octaves));
			curNoiseLayerFile.addField(FileAdapter::FileField("around_zero", FileAdapter::ValueType::BOOL, curNoiseLayer.aroundZero));
			index++;
		}
	}

	void TerrainManager::saveTerrainElements(FileAdapter& file) const {
		FileAdapter& elements = file.getSubElement("terrain_elements");
		elements.clear();
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = this->elements.begin(); it != this->elements.end(); it++) {
			if (!it->getHasDifference()) continue;
			PositionIdentifier posId = it->getPosId();
			std::string key = "x" + std::to_string(posId.x) + "i" + std::to_string(posId.i) + "z" + std::to_string(posId.z) + "n" + std::to_string(posId.n);
			FileAdapter& curElement = elements.getSubElement(key);
			curElement.clear();
			const float* heightArray = it->getDifference();
			std::vector<std::any> heightDifference(heightArray, heightArray + (settings->numWidth * settings->numHeight * 3));
			curElement.addArray(FileAdapter::FileArray("heightDifference", FileAdapter::ValueType::FLOAT, heightDifference));
		}
	}

	void TerrainManager::initialiseAndAddNewElement(std::unordered_set<ManipulableTerrainElement>& newElements, const PositionIdentifier& posId) {
		// Directly emplace a new ManipulableTerrain into the container
		auto result = newElements.emplace(settings, posId);
		if (result.second) { // Check if insertion was successful
			ManipulableTerrainElement& newElement = const_cast<ManipulableTerrainElement&>(*result.first);
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
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
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
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement* element = const_cast<ManipulableTerrainElement*>(&*it); // Const can be cast away since the hash relevant data is not changed
			auto updateNoise = [element]() {
				element->UnloadLayers();
				element->updateNoiseLayers();
				element->randomizeTerrain();
				element->updateNormals();
				element->addDifference();
				};
			if (settings->updateWithThreadPool && settings->threadPool) settings->threadPool->addTask(updateNoise, element->getReloadFlag());
			else {
				updateNoise();
				element->reloadMeshData();
			}
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

	void TerrainManager::loadTerrainElements(const FileAdapter& elements) {
		generateNewManipulableTerrains();

		for (std::string key : elements.getAllSubKeys()) {
			FileAdapter curElementFile = elements.getSubElement(key);
			PositionIdentifier posId = getPositionIdentifierFromKey(key);
			std::unordered_set<ManipulableTerrainElement>::iterator it = this->elements.find(ManipulableTerrainElement(posId));
			if (it == this->elements.end()) continue;
			ManipulableTerrainElement& curElement = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
			curElement.loadDifference(curElementFile.getArray("heightDifference").getValue());
		}

		TraceLog(LOG_DEBUG, "Terrain: Terrain elements have been loaded");
	}

	PositionIdentifier TerrainManager::getPositionIdentifierFromKey(std::string key) {
		PositionIdentifier posId;
		posId.x = std::stoi(key.substr(1, key.find("i") - 1));
		posId.i = std::stoi(key.substr(key.find("i") + 1, key.find("z") - key.find("i") - 1));
		posId.z = std::stoi(key.substr(key.find("z") + 1, key.find("n") - key.find("z") - 1));
		posId.n = std::stoi(key.substr(key.find("n") + 1, key.length() - key.find("n") - 1));
		return posId;
	}

	void TerrainManager::initializeModel() {
		m_model = newModel();

		loadElementsIntoModel();
		initializeModelMaterials();
		updateBoundingBox();

		TraceLog(LOG_DEBUG, "Terrain: Model has been initialized");

		*modelUploaded = true;
	}

	void TerrainManager::loadNoiseSettings(const FileAdapter& settingsFile) {
		noiseSettings = std::make_shared<Noise::noise_settings>(Noise::newNoiseSettings());

		noiseSettings->seed = std::any_cast<int>(settingsFile.getField("seed").getValue());
		
		for (int index = 0; ; index++) {
			FileAdapter curNoiseLayerFile = settingsFile.getSubElement(std::to_string(index));
			if (curNoiseLayerFile.getKey() == "") break;

			Noise::noise_layer_settings curNoiseLayer;
			curNoiseLayer.horizontalScale = std::any_cast<float>(curNoiseLayerFile.getField("horizontal_scale").getValue());
			curNoiseLayer.verticalScale = std::any_cast<float>(curNoiseLayerFile.getField("vertical_scale").getValue());
			curNoiseLayer.offsetX = std::any_cast<int>(curNoiseLayerFile.getField("offset_x").getValue());
			curNoiseLayer.offsetZ = std::any_cast<int>(curNoiseLayerFile.getField("offset_z").getValue());
			curNoiseLayer.lacunarity = std::any_cast<float>(curNoiseLayerFile.getField("lacunarity").getValue());
			curNoiseLayer.gain = std::any_cast<float>(curNoiseLayerFile.getField("gain").getValue());
			curNoiseLayer.octaves = std::any_cast<int>(curNoiseLayerFile.getField("octaves").getValue());
			curNoiseLayer.aroundZero = std::any_cast<bool>(curNoiseLayerFile.getField("around_zero").getValue());
			noiseSettings->noiseLayerSettings.push_back(curNoiseLayer);
		}

		TraceLog(LOG_DEBUG, "Terrain: Noise has been loaded");
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
			std::unordered_set<ManipulableTerrainElement>::iterator start = std::next(elements.begin(), settings->maxNumElements);
		
			// Free memory used by the elements and delete from the set
			for (std::unordered_set<ManipulableTerrainElement>::iterator it = start; it != elements.end();) {
				ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
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

		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
			element.UnloadLayers();
		}

		generateNewManipulableTerrains();
		initializeModel();

		TraceLog(LOG_DEBUG, "Terrain: Terrain has been renewed");
	}

	void TerrainManager::relocateElements() {
		TraceLog(LOG_DEBUG, "Terrain: Relocating elements of terrain");

		std::unordered_set<ManipulableTerrainElement> newElements;

		Vector3 position = { 0.0f, 0.0f, 0.0f };
		if (settings->camera) position = Vector3Subtract(settings->camera->getPosition(), m_position);

		// Spawning elements from the bottom left corner
		float width = (settings->numWidth - 1) * settings->spacing;
		float height = (settings->numHeight - 1) * settings->spacing;
		int numPerQuadrantX = round(settings->radius / width);
		int numPerQuadrantZ = round(settings->radius / height);
		float x = position.x - numPerQuadrantX * width;
		float z = position.z - numPerQuadrantZ * height;
		bool maxElementsReached = false;
		for (int i = -numPerQuadrantX; i < numPerQuadrantX; i++, x += width) {
			float circleHeight = getSpawnHeightAtXPos(x - position.x + (width / 2), settings->radius);
			for (int j = -numPerQuadrantZ; j < numPerQuadrantZ; j++, z += height) {
				if (std::abs(z - position.z + (height / 2)) > circleHeight) continue;

				int i = x < 0 ? -1 : 1;
				int posX = (x - width * std::min(0, i)) / (width * i);
				int n = z < 0 ? -1 : 1;
				int posZ = (z - height * std::min(0, n)) / (height * n);

				// If there is already a element present here, then keep it
				if (elements.size() > 0) {
					ManipulableTerrainElement element({ posX, i, posZ, n });
					auto newElement = elements.extract(element);

					if (!newElement.empty()) {
						newElements.insert(std::move(newElement));
						continue;
					}
				}

				// There is no element already present, so make a new one
				initialiseAndAddNewElement(newElements, { posX, i, posZ, n });

				if (newElements.size() >= settings->maxNumElements) {
					maxElementsReached = true;
					break;
				}
			}
			z = position.z - numPerQuadrantZ * height;
			if (maxElementsReached) break;
		}

		// Set new elements and elements that aren't needed anymore
		elements.clear();
		elements = std::move(newElements);
	}

	void TerrainManager::manipulateTerrain(ManipulableTerrainElement::ManipulateDir dir, ManipulableTerrainElement::ManipulateForm form, ManipulableTerrainElement::ManipulateType type, float strength, float radius, Vector3 position) {
		// TODO: Make it so that not all elements are manipulated, but only the ones that are in the radius of the manipulation
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
			element.manipulateTerrain(dir, form, type, strength, radius, Vector3Subtract(position, element.getPosition()));
		}
	}

	void TerrainManager::update() {
		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it);
			element.update();
		}
	}

	void TerrainManager::draw() {
		// activate();
		ModelObject::draw(m_position);
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

		for (std::unordered_set<ManipulableTerrainElement>::iterator it = elements.begin(); it != elements.end(); it++) {
			ManipulableTerrainElement& element = const_cast<ManipulableTerrainElement&>(*it); // Const can be cast away since the hash relevant data is not changed
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