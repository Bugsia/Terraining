#include "Terrain/TerrainElement.h"

namespace Terrain {
	Vector3 TerrainElement::getPositionFromPosId() {
		float xSize = (settings->numWidth - 1) * settings->spacing;
		float xPos = posId.x * xSize * posId.i + xSize * std::min(0, posId.i);

		float zSize = (settings->numHeight - 1) * settings->spacing;
		float zPos = posId.z * zSize * posId.n + zSize * std::min(0, posId.n);

		return { xPos, 0., zPos };
	}

	float* TerrainElement::flatTerrainVertices() {
		int numVertices = settings->numWidth * settings->numHeight;
		float* vertices = (float*)RL_MALLOC(numVertices * 3 * sizeof(float));

		int index = 0;
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				vertices[index] = static_cast<float>(x * settings->spacing) + position.x;
				vertices[index + 1] = position.y;
				vertices[index + 2] = static_cast<float>(z * settings->spacing) + position.z;

				index += 3;
			}
		}

		m_mesh.vertexCount = numVertices;
		return vertices;
	}

	float* TerrainElement::flatTerrainTexcoords() {
		int numTexcoords = settings->numWidth * settings->numHeight * 2;
		float* texcoords = (float*)RL_MALLOC(numTexcoords * sizeof(float));

		int index = 0;
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				texcoords[index] = static_cast<float>(x) / (settings->numWidth - 1);
				texcoords[index + 1] = static_cast<float>(z) / (settings->numHeight - 1);

				index += 2;
			}
		}

		return texcoords;
	}

	float* TerrainElement::flatTerrainNormals() {
		int numNormals = settings->numWidth * settings->numHeight;
		float* normals = (float*)RL_MALLOC(numNormals * 3 * sizeof(float));

		int index = 0;
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				normals[index] = 0;
				normals[index + 1] = 1;
				normals[index + 2] = 0;
				index += 3;
			}
		}

		return normals;
	}

	unsigned short* TerrainElement::flatTerrainIndices() {
		int numIndices = (settings->numWidth - 1) * (settings->numHeight - 1) * 6;
		unsigned short* indices = (unsigned short*)RL_MALLOC(numIndices * sizeof(unsigned short));

		int index = 0;
		int loopIterations = (numIndices / 6) + (settings->numWidth - 1);
		for (int i = 0; i < loopIterations; i++) {
			if ((i + 1) % settings->numHeight == 0) continue;

			indices[index] = i;
			indices[index + 1] = i + 1;
			indices[index + 2] = i + settings->numHeight;

			indices[index + 3] = i + 1;
			indices[index + 4] = i + settings->numHeight + 1;
			indices[index + 5] = i + settings->numHeight;

			index += 6;
		}

		m_mesh.triangleCount = numIndices / 3;
		return indices;
	}

	template <typename T>
	void TerrainElement::copyVectorToMemory(T*& dst, std::vector<T> src, bool uploaded) {
		// if (!uploaded && dst) RL_FREE(dst); // TODO: Does this bring anything?
		if (!uploaded) dst = (T*)RL_MALLOC(src.size() * sizeof(T));
		memcpy(dst, src.data(), src.size() * sizeof(T));
	}

	void TerrainElement::initialiseFlatMesh() {
		m_mesh.vertices = flatTerrainVertices();
		m_mesh.normals = flatTerrainNormals();
		m_mesh.indices = flatTerrainIndices();
		m_mesh.texcoords = flatTerrainTexcoords();
	}

	int TerrainElement::getIdFromPosId() {
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
		id = getIdFromPosId();
		position = getPositionFromPosId();

		TraceLog(LOG_DEBUG, "TerrainElement: New element %i has been created", id);
	}

	TerrainElement::TerrainElement(PositionIdentifier posId) : posId(posId), id(getIdFromPosId()) {
		TraceLog(LOG_DEBUG, "TerrainElement: New search element %i has been created", id);
	}

	TerrainElement::TerrainElement(const TerrainElement& other) : MeshEntity(other), id(other.id), settings(other.settings), position(other.position), 
			posId(other.posId), dynamicMesh(other.dynamicMesh), meshUploaded(other.meshUploaded), modelUploaded(other.modelUploaded) {
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
	}

	void TerrainElement::initialiseElementWithFlatTerrain() {
		TraceLog(LOG_DEBUG, "TerrainElement: Filling element %i with flat terrain", id);

		initialiseFlatMesh();

		reloadMeshData();
	}

	void TerrainElement::initialiseElementWithNoiseTerrain(std::shared_ptr<Noise::noise_settings> noiseSettings) {
		TraceLog(LOG_DEBUG, "TerrainElement: Filling element %i with noise terrain", id);

		initialiseFlatMesh();

		this->noiseSettings = noiseSettings;
		updateNoiseLayers();

		randomizeTerrain();
		updateNormals();
		updateBoundingBox();

		reloadMeshData();
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
		noiseLayerPixels = Noise::generateNoiseLayers(noiseSettings, position, settings->numWidth, settings->numHeight, settings->spacing, noiseSettings->seed);
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
		position = getPositionFromPosId();
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

	unsigned int TerrainElement::getId() const {
		return id;
	}

	Mesh& TerrainElement::refMesh() {
		return m_mesh;
	}

	void TerrainElement::setModelUploaded(std::shared_ptr<bool> modelUploaded) {
		this->modelUploaded = modelUploaded;
	}
}