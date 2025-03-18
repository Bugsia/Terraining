#pragma once
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "TerrainElement.h"
#include "ManipulableTerrainElement.h"
#include "ModelObject.h"
#include "Actor.h"
#include "Drawable.h"
#include "FileAdapters/FileAdapter.h"

// Custom hash function for TerrainElement
namespace std {
	template<>
	struct hash<Terrain::TerrainElement> {
		size_t operator()(const Terrain::TerrainElement& element) const {
			return std::hash<unsigned int>()(element.getId());
		}
	};

	template<>
	struct hash<Terrain::ManipulableTerrainElement> {
		size_t operator()(const Terrain::ManipulableTerrainElement& element) const {
			return std::hash<unsigned int>()(element.getId());
		}
	};
}

namespace Terrain {
	/**
	* @class BaseTerrainManager
	* @brief Defines the public functions for all TerrainManager.
	*/
	class BaseTerrainManager : public ModelObject, public Actor<Vector3>, public Drawable {
	public:
		virtual ~BaseTerrainManager() = default;
		BaseTerrainManager(std::string name) : Actor<Vector3>(name) {}

		virtual void draw() = 0;
		virtual void update(int targetFPS) = 0;
		virtual void recalculateElementPosition() = 0;
		virtual void recalculateElementNoise() = 0;
		virtual void reloadTerrain() = 0;

		// Collision
		virtual RayCollision getRayCollisionWithTerrain(Ray ray) = 0;

		// Saving and loading
		virtual void save(FileAdapter& file) const = 0;
		virtual bool load(const FileAdapter& file) = 0;

		// Getter and Setter
		virtual void setThreadPool(ThreadPool* threadPool) = 0;
		virtual void setCamera(Character* camera) = 0;

		virtual terrain_settings* getTerrainSettings() = 0;
		virtual Noise::noise_settings* getNoiseSettings() = 0;
	};

	namespace {
		/**
		* @namespace JsonKeys
		* @brief This namespace defines the names of variables and sub elements used for lookup in the FileAdapter
		*/
		namespace JsonKeys {
			// Sub-Elements
			constexpr std::string_view manipulations = "terrain_manipulations";
			constexpr std::string_view manipulationsArray = "height_difference";
			constexpr std::string_view noiseSettings = "noise_settings";
			constexpr std::string_view terrainSettings = "terrain_settings";

			// Variables
			constexpr std::string_view radius = "radius";
			constexpr std::string_view maxNumElements = "max_num_elements";
			constexpr std::string_view followCamera = "follow_camera";
			constexpr std::string_view updateWithThreadPool = "update_with_thread_pool";
			constexpr std::string_view distToRelocating = "dist_to_relocating";
			constexpr std::string_view numWidth = "num_width";
			constexpr std::string_view numHeight = "num_height";
			constexpr std::string_view spacing = "spacing";
		}

		/**
		* @class BaseTerrainManager
		* @brief This class holds all the code that both versions (manipulable and non-manipulable) of the TerrainManager share and is not meant for use. Then the two versions of the TerrainManager inherit from this class.
		* @tparam T Can be TerrainElement or ManipulableTerrainElement.
		*/
		template <typename T>
		concept ElementTypes = std::same_as<T, TerrainElement> || std::same_as<T, ManipulableTerrainElement>;

		template <typename Derived, ElementTypes T>
		class TemplateTerrainManager : public BaseTerrainManager {
		public:
			virtual ~TemplateTerrainManager() = default;
			TemplateTerrainManager(std::string name, terrain_settings terrainSettings, Noise::noise_settings noiseSettings);
			TemplateTerrainManager(std::string name, const FileAdapter& settings);

			void draw();
			void update(int targetFPS);
			void recalculateElementPosition();
			void recalculateElementNoise();
			void reloadTerrain();

			// Collision
			RayCollision getRayCollisionWithTerrain(Ray ray);

			// Saving and loading
			virtual void save(FileAdapter& file) const;
			virtual bool load(const FileAdapter& file);

			// Getter and Setter
			void setThreadPool(ThreadPool* threadPool);
			void setCamera(Character* camera);

			terrain_settings* getTerrainSettings();
			Noise::noise_settings* getNoiseSettings();

		protected:
			terrain_settings m_terrainSettings;
			Noise::noise_settings m_noiseSettings;
			std::unordered_set<T> m_elements;

			bool modelUploaded = false;
			std::atomic<bool> m_updateModel{ false };
			std::mutex m_lockElements;
			Vector3 m_center = { 0.0f, 0.0f, 0.0f };

			TemplateTerrainManager(std::string name);

			// Initilization
			void initializeTerrain();
			void initializeNoise();
			void initializeModel();

			virtual void initialiseAndAddNewElement(std::unordered_set<T>& newElements, const PositionIdentifier& posId) = 0;

			// Settings loading and saving (as an argument give base level settings)
			bool loadNoiseSettings(const FileAdapter& settings);
			bool loadTerrainSettings(const FileAdapter& settings);

			void saveNoiseSettings(FileAdapter& file) const;
			void saveTerrainSettings(FileAdapter& file) const;

			// Helper functions
			void relocateElements();
			void updateModel();
			void loadElementsIntoModel();
			void initializeModelMaterials();
			void updateCenter();
			virtual void reloadElement(T* element) = 0;

			float getSpawnHeightAtXPos(float x, float spawnRadius) const;
			PositionIdentifier getPositionIdentifierFromPosition(Vector3 pos) const;
			std::vector<PositionIdentifier> getPositionIdentifiersInRadius(Vector3 pos, float radius) const;
		};

		template <typename Derived, ElementTypes T>
		TemplateTerrainManager<Derived, T>::TemplateTerrainManager<Derived, T>(std::string name, terrain_settings terrainSettings, Noise::noise_settings noiseSettings) : BaseTerrainManager(name), m_terrainSettings(terrainSettings), m_noiseSettings(noiseSettings) {
			initializeTerrain();

			TraceLog(LOG_DEBUG, "TerrainManager: Constructed");
		}

		template <typename Derived, ElementTypes T>
		TemplateTerrainManager<Derived, T>::TemplateTerrainManager<Derived, T>(std::string name, const FileAdapter& settings) : BaseTerrainManager(name) {
			m_filename = settings.getFilename();
			load(settings);
			initializeTerrain();

			TraceLog(LOG_DEBUG, "TerrainManager: Constructed from file");
		}

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::draw() {
			ModelObject::draw(m_position);
		}

		/**
		* The elements are only updated if m_lockElements can be locked. Otherwise it does nothing.
		* A maximum of 75 percent of the whole framtime is used to update the elements. Is checked after a single element update call, so it may overstep slightly.
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::update(int targetFPS) {
			// Update Elements
			if (!m_lockElements.try_lock()) return;
			double start = GetTime();
			for (const T& constElement : m_elements) {
				T& element = const_cast<T&>(constElement);
				element.update(targetFPS);
				if (GetTime() - start > 1.0f / (targetFPS * 1.33f)) {
					m_lockElements.unlock();
					return;
				}
			}

			// Update Terrain
			if (m_updateModel.load()) {
				updateModel();
				m_updateModel.store(false);
			}
			m_lockElements.unlock();

			if (m_terrainSettings.followCamera) {
				Vector3 camPosition = m_terrainSettings.camera->getPosition();
				if (Vector2Distance({ m_center.x, m_center.z }, { camPosition.x, camPosition.z }) > m_terrainSettings.distToRelocating) recalculateElementPosition();
			}
		}

		/**
		* 1. Deletes / spawns elements according to spawnRadius
		* 2. Deletes / spawns elements according to camera position
		* 3. Deletes / spawns elements according to maxNumElements
		*
		* It should not be run in a seperate thread (if multithreading is enabled all the work is done in a seperate thread anyway)
		* A called function tries to acquire m_lockElements. Function or started Thread will be blocked until lock can be acquired
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::recalculateElementPosition() {
			auto function = [this]() {
				relocateElements();
				};

			if (m_terrainSettings.updateWithThreadPool && m_terrainSettings.threadPool) {
				m_terrainSettings.threadPool->addTask(function, nullptr);
			}
			else {
				function();
			}

			TraceLog(LOG_DEBUG, "TerrainManager: Position recalculated");
		}

		/**
		* It removes the noise of all elements and then reapplies it (So it applies changes in noiseSettings).
		* It runs all the updating in one seperate thread.
		*
		* It should not be run in a seperate thread (if multithreading is enabled all the work is done in a seperate thread anyway)
		* Tried to acquire m_lockElements. Function or started Thread will be blocked until lock can be acquired
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::recalculateElementNoise() {
			auto function = [this]() {
				m_lockElements.lock();
				for (typename std::unordered_set<T>::iterator it = m_elements.begin(); it != m_elements.end(); it++) {
					T* element = const_cast<T*>(&*it); // Const can be cast away since the hash relevant data is not changed
					reloadElement(element);
				}
				m_lockElements.unlock();
				};

			if (m_terrainSettings.updateWithThreadPool && m_terrainSettings.threadPool) {
				m_terrainSettings.threadPool->addTask(function, nullptr);
			}
			else {
				function();
			}

			TraceLog(LOG_DEBUG, "TerrainManager: Noise recalculated");
		}

		/**
		* It removes all terrainElements and then creates them again.
		* Applies changes, that a element wont apply without creating them again, such as spacing.
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::reloadTerrain() {
			m_elements.clear();
			recalculateElementPosition();
		}

		template <typename Derived, ElementTypes T>
		RayCollision TemplateTerrainManager<Derived, T>::getRayCollisionWithTerrain(Ray ray) {
			RayCollision hit = { 0 };
			ray.position = Vector3Subtract(ray.position, m_position);
			if (!GetRayCollisionBox(ray, getBoundingBox()).hit) return hit;
			if (!m_lockElements.try_lock()) return hit;

			for (typename std::unordered_set<T>::iterator it = m_elements.begin(); it != m_elements.end(); it++) {
				T& element = const_cast<T&>(*it); // Const can be cast away since the hash relevant data is not changed
				RayCollision boundingBoxHit = GetRayCollisionBox(ray, element.getBoundingBox());
				if (boundingBoxHit.hit) {
					RayCollision elementHit = GetRayCollisionMesh(ray, element.refMesh(), m_model.transform);
					if (elementHit.hit) {
						hit = elementHit;
						break;
					}
				}
			}

			m_lockElements.unlock();
			return hit;
		}

		/**
		* @params file Should be the base level. The function will append the terrain as a sub element.
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::save(FileAdapter& file) const {
			FileAdapter terrain = file.getSubElement(m_name);
			Actor::save(terrain);
			saveTerrainSettings(terrain);
			saveNoiseSettings(terrain);

			TraceLog(LOG_DEBUG, "TerrainManager: Saved to file %s", file.getFilename().c_str());
		}

		/**
		* @params file File should have a sub element with the name of the TerrainManager
		* @return True if all required fields were present, false otherwise
		*/
		template <typename Derived, ElementTypes T>
		bool TemplateTerrainManager<Derived, T>::load(const FileAdapter& file) {
			bool success = true;

			FileAdapter terrain = file.getSubElement(m_name);
			if (terrain.getKey() == "") success = false;
			if (success) success = Actor::load(terrain) && loadTerrainSettings(terrain) && loadNoiseSettings(terrain);

			if (success) TraceLog(LOG_DEBUG, "TerrainManager: Loaded from file %s", file.getFilename().c_str());
			else TraceLog(LOG_WARNING, "TerrainManager: Loading failed from file %s", file.getFilename().c_str());

			return success;
		}

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::setThreadPool(ThreadPool* threadPool) {
			m_terrainSettings.threadPool = threadPool;

			TraceLog(LOG_DEBUG, "TerrainManager: ThreadPool set");
		}

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::setCamera(Character* camera) {
			m_terrainSettings.camera = camera;
			if (m_terrainSettings.followCamera && camera) recalculateElementPosition();

			TraceLog(LOG_DEBUG, "TerrainManager: Camera set");
		}

		template <typename Derived, ElementTypes T>
		terrain_settings* TemplateTerrainManager<Derived, T>::getTerrainSettings() {
			return &m_terrainSettings;
		}

		template <typename Derived, ElementTypes T>
		Noise::noise_settings* TemplateTerrainManager<Derived, T>::getNoiseSettings() {
			return &m_noiseSettings;
		}

		/**
		* This constructor does not initialize the Terrain. It is for derived classes, that need to initialize their own variables before Terrain creation.
		* Thus it is protected and should not be called to fully initialize the TerrainManager.
		*/
		template <typename Derived, ElementTypes T>
		TemplateTerrainManager<Derived, T>::TemplateTerrainManager<Derived, T>(std::string name) : BaseTerrainManager(name) { }

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::initializeTerrain() {
			initializeModel();
			recalculateElementPosition();

			TraceLog(LOG_DEBUG, "TerrainManager: Terrain initialized");
		}

		/**
		* Sets the noiseSettings to the default values
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::initializeNoise() {
			m_noiseSettings = Noise::getDefaultNoiseSettings();

			TraceLog(LOG_DEBUG, "TerrainManager: Noise initialized");
		}

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::initializeModel() {
			loadElementsIntoModel();
			initializeModelMaterials();
			updateBoundingBox();

			modelUploaded = true;

			TraceLog(LOG_DEBUG, "TerrainManager: Model initialized");
		}

		/**
		* Will maybe be moved to Noise namespace in the future
		*
		* @params settings Should be the base level. The function will search for the noise Settings as a sub element.
		*/
		template <typename Derived, ElementTypes T>
		bool TemplateTerrainManager<Derived, T>::loadNoiseSettings(const FileAdapter& settings) {
			const FileAdapter& noise = settings.getSubElement(std::string(JsonKeys::noiseSettings));
			if (noise.getKey() == "") {
				TraceLog(LOG_WARNING, "TerrainManager: Noise settings not found in file %s", settings.getFilename());
				return false;
			}

			try {
				m_noiseSettings.seed = std::any_cast<int>(noise.getField("seed").getValue());

				for (int index = 0; ; index++) {
					const FileAdapter curNoiseLayerFile = noise.getSubElement(std::to_string(index));
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
					m_noiseSettings.noiseLayerSettings.push_back(curNoiseLayer);
				}
			}
			catch (std::bad_any_cast& e) {
				TraceLog(LOG_WARNING, "TerrainManager: bad_any_cast in loadNosieSettings: %s", e.what());
				m_noiseSettings = Noise::noise_settings();
				return false;
			}

			TraceLog(LOG_DEBUG, "TerrainManager: Noise settings have been loaded from file");
			return true;
		}


		/**
		* @params settings Should be the base level. The function will search for the terrain Settings as a sub element.
		*/
		template <typename Derived, ElementTypes T>
		bool TemplateTerrainManager<Derived, T>::loadTerrainSettings(const FileAdapter& settings) {
			const FileAdapter& terrain = settings.getSubElement(std::string(JsonKeys::terrainSettings));
			if (terrain.getKey() == "") {
				TraceLog(LOG_WARNING, "TerrainManager: Terrain settings not found in file %s", settings.getFilename());
				return false;
			}

			try {
				m_terrainSettings.radius = std::any_cast<float>(terrain.getField(std::string(JsonKeys::radius)).getValue());
				m_terrainSettings.numWidth = std::any_cast<int>(terrain.getField(std::string(JsonKeys::numWidth)).getValue());
				m_terrainSettings.numHeight = std::any_cast<int>(terrain.getField(std::string(JsonKeys::numHeight)).getValue());
				m_terrainSettings.maxNumElements = std::any_cast<int>(terrain.getField(std::string(JsonKeys::maxNumElements)).getValue());
				m_terrainSettings.spacing = std::any_cast<float>(terrain.getField(std::string(JsonKeys::spacing)).getValue());
				m_terrainSettings.updateWithThreadPool = std::any_cast<bool>(terrain.getField(std::string(JsonKeys::updateWithThreadPool)).getValue());
				m_terrainSettings.followCamera = std::any_cast<bool>(terrain.getField(std::string(JsonKeys::followCamera)).getValue());
				m_terrainSettings.distToRelocating = std::any_cast<float>(terrain.getField(std::string(JsonKeys::distToRelocating)).getValue());
			}
			catch (std::bad_any_cast& e) {
				TraceLog(LOG_WARNING, "TerrainManager: bad_any_cast in loadTerrainSettings: %s", e.what());
				m_terrainSettings = terrain_settings();
				return false;
			}

			TraceLog(LOG_DEBUG, "TerrainManager: Terrain settings have been loaded from file");
			return true;
		}

		/**
		* @params file Should be the base level. The function will add the noise Settings as a sub element.
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::saveNoiseSettings(FileAdapter& file) const {
			FileAdapter& noise = file.getSubElement(std::string(JsonKeys::noiseSettings));
			noise.addField(FileAdapter::FileField("seed", FileAdapter::STRING, m_noiseSettings.seed));

			for (int index = 0; index < m_noiseSettings.noiseLayerSettings.size(); index++) {
				const Noise::noise_layer_settings& curNoiseLayer = m_noiseSettings.noiseLayerSettings[index];
				FileAdapter& curNoiseLayerFile = noise.getSubElement(std::to_string(index));
				curNoiseLayerFile.addField(FileAdapter::FileField("horizontal_scale", FileAdapter::FLOAT, curNoiseLayer.horizontalScale));
				curNoiseLayerFile.addField(FileAdapter::FileField("vertical_scale", FileAdapter::FLOAT, curNoiseLayer.verticalScale));
				curNoiseLayerFile.addField(FileAdapter::FileField("offset_x", FileAdapter::INT, curNoiseLayer.offsetX));
				curNoiseLayerFile.addField(FileAdapter::FileField("offset_z", FileAdapter::INT, curNoiseLayer.offsetZ));
				curNoiseLayerFile.addField(FileAdapter::FileField("lacunarity", FileAdapter::FLOAT, curNoiseLayer.lacunarity));
				curNoiseLayerFile.addField(FileAdapter::FileField("gain", FileAdapter::FLOAT, curNoiseLayer.gain));
				curNoiseLayerFile.addField(FileAdapter::FileField("octaves", FileAdapter::INT, curNoiseLayer.octaves));
				curNoiseLayerFile.addField(FileAdapter::FileField("around_zero", FileAdapter::BOOL, curNoiseLayer.aroundZero));
			}

			TraceLog(LOG_DEBUG, "TerrainManager: Noise settings have been saved to file %s", file.getFilename().c_str());
		}

		/**
		* @params file Should be the base level. The function will add the terrain Settings as a sub element.
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::saveTerrainSettings(FileAdapter& file) const {
			FileAdapter& terrain = file.getSubElement(std::string(JsonKeys::terrainSettings));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::radius), FileAdapter::FLOAT, m_terrainSettings.radius));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::numWidth), FileAdapter::INT, m_terrainSettings.numWidth));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::numHeight), FileAdapter::INT, m_terrainSettings.numHeight));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::maxNumElements), FileAdapter::INT, m_terrainSettings.maxNumElements));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::spacing), FileAdapter::FLOAT, m_terrainSettings.spacing));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::updateWithThreadPool), FileAdapter::BOOL, m_terrainSettings.updateWithThreadPool));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::followCamera), FileAdapter::BOOL, m_terrainSettings.followCamera));
			terrain.addField(FileAdapter::FileField(std::string(JsonKeys::distToRelocating), FileAdapter::FLOAT, m_terrainSettings.distToRelocating));

			TraceLog(LOG_DEBUG, "TerrainManager: Terrain settings have been saved to file %s", file.getFilename().c_str());
		}

		/**
		* Has the logic behind recalculateElementPosition().
		* Can be called in a seperate thread.
		* It will block until it can acquire a lock on m_lockElements
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::relocateElements() {
			std::unordered_set<T> newElements;

			updateCenter();

			std::vector<PositionIdentifier> posIds = getPositionIdentifiersInRadius(m_center, m_terrainSettings.radius);

			m_lockElements.lock();
			for (PositionIdentifier posId : posIds) {
				// Check for an already existing element there and use it, if it exists
				if (m_elements.size() > 0) {
					auto newElement = m_elements.extract(T(posId));
					if (!newElement.empty()) {
						newElements.insert(std::move(newElement));
						continue;
					}
				}
				// There is no element already present, so make a new one
				initialiseAndAddNewElement(newElements, posId);

				if (newElements.size() > m_terrainSettings.maxNumElements) break;
			}

			// Set new elements and elements that aren't needed anymore
			m_elements.clear();
			m_elements = std::move(newElements);
			m_lockElements.unlock();
			m_updateModel.store(true);

			TraceLog(LOG_DEBUG, "TerrainManager: Elements have been relocated");
		}

		/**
		* It builds up the model from scratch and thus adds new elements and removes old ones.
		* Should not be called from a seperate thread
		*/
		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::updateModel() {
			RL_FREE(m_model.meshes);
			RL_FREE(m_model.materials);
			RL_FREE(m_model.meshMaterial);
			loadElementsIntoModel();
			initializeModelMaterials();
			updateBoundingBox();
		}

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::loadElementsIntoModel() {
			m_model.meshCount = m_elements.size();
			m_model.meshes = (Mesh*)RL_CALLOC(m_model.meshCount, sizeof(Mesh));

			int index = 0;
			for (typename std::unordered_set<T>::iterator it = m_elements.begin(); it != m_elements.end(); it++) {
				T& element = const_cast<T&>(*it);
				m_model.meshes[index] = element.refMesh();
				index++;
			}
		}

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::initializeModelMaterials() {
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

		template <typename Derived, ElementTypes T>
		void TemplateTerrainManager<Derived, T>::updateCenter() {
			if (!m_terrainSettings.followCamera || !m_terrainSettings.camera) {
				m_center = { 0.0f, 0.0f, 0.0f };
				return;
			}

			m_center = Vector3Subtract(m_terrainSettings.camera->getPosition(), m_position);
		}

		/**
		* @params x The x position relative to the center
		* @return The height of the spawn radius at a given x position
		*/
		template <typename Derived, ElementTypes T>
		float TemplateTerrainManager<Derived, T>::getSpawnHeightAtXPos(float x, float spawnRadius) const {
			return std::max(0., sqrt(pow(spawnRadius, 2) - pow(x, 2)));
		}

		template <typename Derived, ElementTypes T>
		PositionIdentifier TemplateTerrainManager<Derived, T>::getPositionIdentifierFromPosition(Vector3 pos) const {
			PositionIdentifier posId;
			float width = (m_terrainSettings.numWidth - 1) * m_terrainSettings.spacing;
			float height = (m_terrainSettings.numHeight - 1) * m_terrainSettings.spacing;
			if (pos.x < 0) {
				posId.i = -1;
				posId.x = static_cast<int>(((-pos.x) - width) / width);
			}
			else {
				posId.i = 1;
				posId.x = static_cast<int>(pos.x / width);
			}
			if (pos.z < 0) {
				posId.n = -1;
				posId.z = static_cast<int>(((-pos.z) - height) / height);
			}
			else {
				posId.n = 1;
				posId.z = static_cast<int>(pos.z / height);
			}
			return posId;
		}

		/**
		* @params pos Should be relative to the center of the terrain
		*/
		template <typename Derived, ElementTypes T>
		std::vector<PositionIdentifier> TemplateTerrainManager<Derived, T>::getPositionIdentifiersInRadius(Vector3 pos, float radius) const {
			std::vector<PositionIdentifier> posIds;
			
			float width = (m_terrainSettings.numWidth - 1) * m_terrainSettings.spacing;
			float height = (m_terrainSettings.numHeight - 1) * m_terrainSettings.spacing;
			int numPerQuadrantX = round(radius / width);
			int numPerQuadrantZ = round(radius / height);
			float x = pos.x - numPerQuadrantX * width;
			float z = pos.z - numPerQuadrantZ * height;
			bool maxElementsReached = false;
			for (int i = 0; i < numPerQuadrantX * 2; i++, x += width) {
				float circleHeight = getSpawnHeightAtXPos(x - pos.x + (width / 2), m_terrainSettings.radius);
				for (int j = 0; j < numPerQuadrantZ * 2; j++, z += height) {
					if (std::abs(z - pos.z + (height / 2)) > circleHeight) continue;

					posIds.push_back(getPositionIdentifierFromPosition({ x, 0.0f, z }));
				}
				z = pos.z - numPerQuadrantZ * height;
			}

			return posIds;
		}
	}
	
	class TerrainManager : public TemplateTerrainManager<TerrainManager, TerrainElement> {
		TerrainManager(std::string name, terrain_settings terrainSettings, Noise::noise_settings noiseSettings);
		TerrainManager(std::string name, const FileAdapter& settings);

	private:
		void initialiseAndAddNewElement(std::unordered_set<TerrainElement>& newElements, const PositionIdentifier& posId) override;
		void reloadElement(TerrainElement* element) override;
	};
}