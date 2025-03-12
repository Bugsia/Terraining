#include "Terrain/TerrainElement.h"
#include <chrono>

namespace Terrain {
	Vector3 TerrainElement::getPositionFromPosId() {
		float xSize = (settings->numWidth - 1) * settings->spacing;
		float xPos = posId.x * xSize * posId.i + xSize * std::min(0, posId.i);

		float zSize = (settings->numHeight - 1) * settings->spacing;
		float zPos = posId.z * zSize * posId.n + zSize * std::min(0, posId.n);

		return { xPos, 0., zPos };
	}

	void TerrainElement::flatTerrainVertices() {
		int index = 0;
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				m_mesh.vertices[index] = static_cast<float>(x * settings->spacing) + m_position.x;
				m_mesh.vertices[index + 1] = m_position.y;
				m_mesh.vertices[index + 2] = static_cast<float>(z * settings->spacing) + m_position.z;

				index += 3;
			}
		}
	}

	void TerrainElement::flatTerrainTexcoords() {
		int index = 0;
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				m_mesh.texcoords[index] = static_cast<float>(x) / (settings->numWidth - 1);
				m_mesh.texcoords[index + 1] = static_cast<float>(z) / (settings->numHeight - 1);

				index += 2;
			}
		}
	}

	void TerrainElement::flatTerrainNormals() {
		int index = 0;
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				m_mesh.normals[index] = 0;
				m_mesh.normals[index + 1] = 1;
				m_mesh.normals[index + 2] = 0;
				index += 3;
			}
		}
	}

	void TerrainElement::flatTerrainIndices() {
		int index = 0;
		int loopIterations = ((settings->numWidth - 1) * (settings->numHeight - 1)) + (settings->numWidth - 1);
		for (int i = 0; i < loopIterations; i++) {
			if ((i + 1) % settings->numHeight == 0) continue;

			m_mesh.indices[index] = i;
			m_mesh.indices[index + 1] = i + 1;
			m_mesh.indices[index + 2] = i + settings->numHeight;

			m_mesh.indices[index + 3] = i + 1;
			m_mesh.indices[index + 4] = i + settings->numHeight + 1;
			m_mesh.indices[index + 5] = i + settings->numHeight;

			index += 6;
		}
	}

	template <typename T>
	void TerrainElement::copyVectorToMemory(T*& dst, std::vector<T> src, bool uploaded) {
		// if (!uploaded && dst) RL_FREE(dst); // TODO: Does this bring anything?
		if (!uploaded) dst = (T*)RL_MALLOC(src.size() * sizeof(T));
		memcpy(dst, src.data(), src.size() * sizeof(T));
	}

	void TerrainElement::initialiseFlatMesh() {
		flatTerrainVertices();
		flatTerrainNormals();
		flatTerrainIndices();
		flatTerrainTexcoords();
	}

	int TerrainElement::getIdFromPosId(PositionIdentifier posId) {
		// Since x and z in posId are only in top right quadrant this calculates actual x and z index
		int absXIndex = (posId.x * posId.i) + (std::min(0, posId.i));
		int absZIndex = (posId.z * posId.n) + (std::min(0, posId.n));

		// Cantor pairing function with ability to handle negative numbers
		if (absXIndex < 0) absXIndex = (-2 * absXIndex) - 1;
		else absXIndex *= 2;

		if (absZIndex < 0) absZIndex = (-2 * absZIndex) - 1;
		else absZIndex *= 2;

		return (((absXIndex + absZIndex) * (absXIndex + absZIndex + 1)) / 2) + absZIndex;
	}

	TerrainElement::~TerrainElement() {
		Unload();
	}

	TerrainElement::TerrainElement(std::shared_ptr<terrain_settings> settings, PositionIdentifier posId) : settings(settings), posId(posId), meshUploaded(false) {
		id = getIdFromPosId(posId);
		m_position = getPositionFromPosId();

		TraceLog(LOG_DEBUG, "TerrainElement: New element %i has been created", id);
	}

	TerrainElement::TerrainElement(PositionIdentifier posId) : posId(posId), id(getIdFromPosId(posId)) {
		TraceLog(LOG_DEBUG, "TerrainElement: New search element %i has been created", id);
	}

	TerrainElement::TerrainElement(const TerrainElement& other) : MeshObject(other), id(other.id), settings(other.settings), posId(other.posId), dynamicMesh(other.dynamicMesh), meshUploaded(other.meshUploaded), modelUploaded(other.modelUploaded) {
		noiseLayerPixels = std::vector<Color*>(other.noiseLayerPixels.size(), nullptr);
		for (int i = 0; i < other.noiseLayerPixels.size(); i++) {
			noiseLayerPixels[i] = (Color*)RL_MALLOC(sizeof(Color) * settings->numWidth * settings->numHeight);
			memcpy(noiseLayerPixels[i], other.noiseLayerPixels[i], sizeof(Color) * settings->numWidth * settings->numHeight);
		}
	}

	void TerrainElement::initialiseMesh() {
		TraceLog(LOG_DEBUG, "TerrainElement: Initialising mesh of element %i", id);

		m_mesh = { 0 };
		meshUploaded = false;
		m_mesh.vertices = (float*)RL_MALLOC(settings->numWidth * settings->numHeight * 3 * sizeof(float));
		m_mesh.vertexCount = settings->numWidth * settings->numHeight;
		m_mesh.indices = (unsigned short*)RL_MALLOC((settings->numWidth - 1) * (settings->numHeight - 1) * 6 * sizeof(unsigned short));
		m_mesh.triangleCount = (settings->numWidth - 1) * (settings->numHeight - 1) * 2;
		m_mesh.normals = (float*)RL_MALLOC(settings->numWidth * settings->numHeight * 3 * sizeof(float));
		m_mesh.texcoords = (float*)RL_MALLOC(settings->numWidth * settings->numHeight * 2 * sizeof(float));
	}

	void TerrainElement::initialiseElementWithFlatTerrain() {
		TraceLog(LOG_DEBUG, "TerrainElement: Filling element %i with flat terrain", id);

		initialiseFlatMesh();

		reloadMeshData();
	}

	void TerrainElement::initialiseElementWithNoiseTerrain(std::shared_ptr<Noise::noise_settings> noiseSettings) {
		TraceLog(LOG_DEBUG, "TerrainElement: Filling element %i with noise terrain", id);

		this->noiseSettings = noiseSettings;
		auto initialise = [&]() {
			initialiseFlatMesh();
			updateNoiseLayers();
			randomizeTerrain();
			updateNormals();
			updateBoundingBox();
			};

		if (settings->updateWithThreadPool && settings->threadPool) {
			settings->threadPool->addTask(initialise, &m_reload);
		}
		else {
			initialise();
			m_reload.store(true);
		}
	}

	void TerrainElement::Upload() {
		TraceLog(LOG_DEBUG, "TerrainElement: Uploading element %i", id);

		UploadMesh(&m_mesh, dynamicMesh);
		meshUploaded = true;
	}

	void TerrainElement::Unload() {
		TraceLog(LOG_DEBUG, "TerrainElement: Unloaded element %i", id);

		if (meshUploaded && *modelUploaded) UnloadMesh(m_mesh); // BETTER WAY TO DECIDE WHEN TO UNLOAD. BEST WOULD BE IF UNLOAD MODEL IS CALLED MESH UPLOADED IS SET TO FALSE FOR EVERYONE
		UnloadLayers();

		meshUploaded = false;
	}

	void TerrainElement::UnloadLayers() {
		TraceLog(LOG_DEBUG, "TerrainElement: Unloaded layers of element %i", id);

		for (std::vector<Color*>::iterator it = noiseLayerPixels.begin(); it != noiseLayerPixels.end();) {
			if ((*it)) RL_FREE((*it));
			it = noiseLayerPixels.erase(it);
		}
	}

	void TerrainElement::updateNoiseLayers() {
		noiseLayerPixels = Noise::generateNoiseLayers(noiseSettings, m_position, settings->numWidth, settings->numHeight, settings->spacing, noiseSettings->seed);
	}

	void TerrainElement::randomizeTerrain() {
		TraceLog(LOG_DEBUG, "TerrainElement: Randomizing terrain of element %i", id);

		int numVertices = m_mesh.vertexCount * 3;
		for (int i = 0; i < numVertices; i += 3) {
			int indexX = (i / 3 / settings->numHeight);
			int indexZ = (i / 3 % settings->numHeight);
			m_mesh.vertices[i + 1] = Noise::noiseHeight(noiseLayerPixels, noiseSettings->noiseLayerSettings, indexX, indexZ, settings->numWidth);
		}
	}

	void TerrainElement::updatePosition() {
		m_position = getPositionFromPosId();
	}

	void TerrainElement::updateNormals() {
		// TODO: IMPLEMENT
	}

	void TerrainElement::reloadMeshData() {
		TraceLog(LOG_DEBUG, "TerrainElement: Updating mesh data of element %i", id);

		bool meshUploaded = this->meshUploaded && modelUploaded;

		if (!meshUploaded) return;

		UpdateMeshBuffer(m_mesh, 0, m_mesh.vertices, m_mesh.vertexCount * 3 * sizeof(float), 0);
		UpdateMeshBuffer(m_mesh, 6, m_mesh.indices, m_mesh.triangleCount * 3 * sizeof(unsigned short), 0);
		UpdateMeshBuffer(m_mesh, 2, m_mesh.normals, m_mesh.vertexCount * 3 * sizeof(float), 0);
		UpdateMeshBuffer(m_mesh, 1, m_mesh.texcoords, m_mesh.vertexCount * 2 * sizeof(float), 0);

		updateBoundingBox();
	}

	void TerrainElement::renewMeshData() {
		TraceLog(LOG_DEBUG, "Terrain Element: Renewing mesh data of element %i", id);

		if (meshUploaded && modelUploaded) {
			UnloadMesh(m_mesh);
			meshUploaded = false;
		}

		initialiseMesh();
		initialiseElementWithNoiseTerrain(noiseSettings);
	}

	void TerrainElement::update() {
		if (m_reload.load()) {
			reloadMeshData();
			m_reload.store(false);
		}
	}

	unsigned int TerrainElement::getId() const {
		return id;
	}

	PositionIdentifier TerrainElement::getPosId() const {
		return posId;
	}

	Mesh& TerrainElement::refMesh() {
		return m_mesh;
	}

	void TerrainElement::setModelUploaded(std::shared_ptr<bool> modelUploaded) {
		this->modelUploaded = modelUploaded;
	}

	std::atomic<bool>* TerrainElement::getReloadFlag() {
		return &m_reload;
	}
}