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

	Vector3 TerrainElement::getVertexFromIndex(int index, float* vertices) {
		return { vertices[index * 3], vertices[index * 3 + 1], vertices[index * 3 + 2] };
	}

	void TerrainElement::addNormalToVertex(Vector3 normal, int index) {
		m_mesh.normals[index * 3] += normal.x;
		m_mesh.normals[index * 3 + 1] += normal.y;
		m_mesh.normals[index * 3 + 2] += normal.z;
	}

	//float* TerrainElement::getExtendedVertices(const std::array<TerrainElement*, 8> neighbours) {
	//	// Create extended vertex array with one extra row/column on each side
	//	int extendedWidth = settings->numWidth + 2;
	//	int extendedHeight = settings->numHeight + 2;
	//	float* vertices = new float[extendedWidth * extendedHeight * 3];

	//	// Initialize to zero
	//	std::memset(vertices, 0, extendedWidth * extendedHeight * 3 * sizeof(float));

	//	// Top left corner (i = 0)
	//	if (neighbours[0]) {
	//		int srcIdx = ((settings->numWidth - 1) * settings->numHeight - 2) * 3;
	//		int dstIdx = 0;
	//		for (int i = 0; i < 3; i++) {
	//			vertices[dstIdx + i] = neighbours[0]->m_mesh.vertices[srcIdx + i];
	//		}
	//	}

	//	// Top edge (i = 1)
	//	if (neighbours[1]) {
	//		for (int x = 0; x < settings->numWidth; x++) {
	//			int srcIdx = ((x + 1) * settings->numHeight - 2) * 3;
	//			int dstIdx = ((x + 1) * extendedHeight) * 3;
	//			for (int i = 0; i < 3; i++) {
	//				vertices[dstIdx + i] = neighbours[1]->m_mesh.vertices[srcIdx + i];
	//			}
	//		}
	//	}

	//	// Top right corner (i = 2)
	//	if (neighbours[2]) {
	//		int srcIdx = (2 * settings->numHeight - 2) * 3;
	//		int dstIdx = ((settings->numWidth + 1) * extendedHeight) * 3;
	//		for (int i = 0; i < 3; i++) {
	//			vertices[dstIdx + i] = neighbours[2]->m_mesh.vertices[srcIdx + i];
	//		}
	//	}

	//	// Left edge (i = 3)
	//	if (neighbours[3]) {
	//		for (int z = 0; z < settings->numHeight; z++) {
	//			int srcIdx = ((settings->numWidth - 2) * settings->numHeight + z) * 3;
	//			int dstIdx = (z + 1) * 3;
	//			for (int i = 0; i < 3; i++) {
	//				vertices[dstIdx + i] = neighbours[3]->m_mesh.vertices[srcIdx + i];
	//			}
	//		}
	//	}

	//	// Center (this)
	//	for (int x = 0; x < settings->numWidth; x++) {
	//		for (int z = 0; z < settings->numHeight; z++) {
	//			int srcIdx = (x * settings->numHeight + z) * 3;
	//			int dstIdx = ((x + 1) * extendedHeight + (z + 1)) * 3;
	//			for (int i = 0; i < 3; i++) {
	//				vertices[dstIdx + i] = m_mesh.vertices[srcIdx + i];
	//			}
	//		}
	//	}

	//	// Right edge (i = 4)
	//	if (neighbours[4]) {
	//		for (int z = 0; z < settings->numHeight; z++) {
	//			int srcIdx = (settings->numHeight + z) * 3;
	//			int dstIdx = ((settings->numWidth + 1) * extendedHeight + z + 1) * 3;
	//			for (int i = 0; i < 3; i++) {
	//				vertices[dstIdx + i] = neighbours[4]->m_mesh.vertices[srcIdx + i];
	//			}
	//		}
	//	}

	//	// Bottom left corner (i = 5)
	//	if (neighbours[5]) {
	//		int srcIdx = (((settings->numWidth - 2) * settings->numHeight) + 1) * 3;
	//		int dstIdx = (extendedHeight - 1) * 3;
	//		for (int i = 0; i < 3; i++) {
	//			vertices[dstIdx + i] = neighbours[5]->m_mesh.vertices[srcIdx + i];
	//		}
	//	}

	//	// Bottom edge (i = 6)
	//	if (neighbours[6]) {
	//		for (int x = 0; x < settings->numWidth; x++) {
	//			int srcIdx = (x * settings->numHeight + 1) * 3;
	//			int dstIdx = ((x + 2) * extendedHeight - 1) * 3;
	//			for (int i = 0; i < 3; i++) {
	//				vertices[dstIdx + i] = neighbours[6]->m_mesh.vertices[srcIdx + i];
	//			}
	//		}
	//	}

	//	// Bottom right corner (i = 7)
	//	if (neighbours[7]) {
	//		int srcIdx = settings->numHeight + 1;
	//		int dstIdx = (extendedWidth * extendedHeight - 1) * 3;
	//		for (int i = 0; i < 3; i++) {
	//			vertices[dstIdx + i] = neighbours[7]->m_mesh.vertices[srcIdx + i];
	//		}
	//	}

	//	return vertices;
	//}

	float* TerrainElement::getExtendedVertices(const std::array<TerrainElement*, 8> neighbours) {
		int extendedWidth = settings->numWidth + 2;
		int extendedHeight = settings->numHeight + 2;
		size_t bufferSize = extendedWidth * extendedHeight * 3;
		float* vertices = new float[bufferSize];
	
		// Initialize to zero
		std::memset(vertices, 0, bufferSize * sizeof(float));
	
		// Top left corner (i = 0)
		if (neighbours[0]) {
			int srcIdx = ((settings->numWidth - 2) * settings->numHeight + (settings->numHeight - 2)) * 3;
			int dstIdx = 0;
			for (int i = 0; i < 3; i++) {
				vertices[dstIdx + i] = neighbours[0]->m_mesh.vertices[srcIdx + i];
			}
		}

		// Left edge (i = 1)
		if (neighbours[1]) {
			for (int z = 0; z < settings->numHeight; z++) {
				int srcIdx = ((settings->numWidth - 2) * settings->numHeight + z) * 3;
				int dstIdx = (z + 1) * 3;
				for (int i = 0; i < 3; i++) {
					vertices[dstIdx + i] = neighbours[1]->m_mesh.vertices[srcIdx + i];
				}
			}
		}

		// Bottom left corner (i = 2)
		if (neighbours[2]) {
			int srcIdx = ((settings->numWidth - 2) * settings->numHeight + 1) * 3;
			int dstIdx = (extendedHeight - 1) * 3;
			for (int i = 0; i < 3; i++) {
				vertices[dstIdx + i] = neighbours[2]->m_mesh.vertices[srcIdx + i];
			}
		}

		// Top edge (i = 3)
		if (neighbours[3]) {
			for (int x = 0; x < settings->numWidth; x++) {
				int srcIdx = (x * settings->numHeight + settings->numHeight - 2) * 3;
				int dstIdx = ((x + 1) * extendedHeight) * 3;
				for (int i = 0; i < 3; i++) {
					vertices[dstIdx + i] = neighbours[3]->m_mesh.vertices[srcIdx + i];
				}
			}
		}

		// Center (this)
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				int srcIdx = (x * settings->numHeight + z) * 3;
				int dstIdx = ((x + 1) * extendedHeight + (z + 1)) * 3;
				for (int i = 0; i < 3; i++) {
					vertices[dstIdx + i] = m_mesh.vertices[srcIdx + i];
				}
			}
		}

		// Bottom edge (i = 4)
		if (neighbours[4]) {
			for (int x = 0; x < settings->numWidth; x++) {
				int srcIdx = (x * settings->numHeight + 1) * 3;
				int dstIdx = ((x + 1) * extendedHeight + extendedHeight - 1) * 3;
				for (int i = 0; i < 3; i++) {
					vertices[dstIdx + i] = neighbours[4]->m_mesh.vertices[srcIdx + i];
				}
			}
		}
	
		// Top right corner (i = 5)
		if (neighbours[5]) {
			int srcIdx = (settings->numHeight + settings->numHeight - 2) * 3;
			int dstIdx = ((settings->numWidth + 1) * extendedHeight) * 3;
			for (int i = 0; i < 3; i++) {
				vertices[dstIdx + i] = neighbours[5]->m_mesh.vertices[srcIdx + i];
			}
		}
	
		// Right edge (i = 6)
		if (neighbours[6]) {
			for (int z = 0; z < settings->numHeight; z++) {
				int srcIdx = (settings->numHeight + z) * 3;
				int dstIdx = ((settings->numWidth + 1) * extendedHeight + (z + 1)) * 3;
				for (int i = 0; i < 3; i++) {
					vertices[dstIdx + i] = neighbours[6]->m_mesh.vertices[srcIdx + i];
				}
			}
		}

		// Bottom right corner (i = 7)
		if (neighbours[7]) {
			int srcIdx = (settings->numHeight + 1) * 3;
			int dstIdx = (extendedWidth * extendedHeight - 1) * 3;
			for (int i = 0; i < 3; i++) {
				vertices[dstIdx + i] = neighbours[7]->m_mesh.vertices[srcIdx + i];
			}
		}
	
		return vertices;
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

	TerrainElement::TerrainElement(terrain_settings* settings, PositionIdentifier posId) : settings(settings), posId(posId), meshUploaded(false) {
		id = getIdFromPosId(posId);
		m_position = getPositionFromPosId();

		TraceLog(LOG_DEBUG, "TerrainElement: New element %i has been created", id);
	}

	TerrainElement::TerrainElement(PositionIdentifier posId) : posId(posId) {
		id = getIdFromPosId(posId);
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

	void TerrainElement::initialiseElementWithNoiseTerrain(Noise::noise_settings* noiseSettings) {
		TraceLog(LOG_DEBUG, "TerrainElement: Filling element %i with noise terrain", id);

		this->noiseSettings = noiseSettings;
		initialiseFlatMesh();
		updateNoiseLayers();
		randomizeTerrain();
		updateNormals();
		updateBoundingBox();
		m_reload.store(true);
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
		// Reset normals to 0
		std::memset(m_mesh.normals, 0, m_mesh.vertexCount * 3 * sizeof(float));

		float* vertices = getExtendedVertices(m_neighbours);

		// Looping through every vertex and then calculating the normal of it (it might be better to look at every triangle but there are more triangle than those in m_mesh.indices, since we are looking at the vertices one out aswell)
		for (int x = 0; x < settings->numWidth; x++) {
			for (int z = 0; z < settings->numHeight; z++) {
				int index = x * settings->numHeight + z;
				int extendedIndex = (x + 1) * (settings->numHeight + 2) + (z + 1);

				// Vertices
				Vector3 a = getVertexFromIndex(extendedIndex, vertices);
				Vector3 b = getVertexFromIndex(extendedIndex - 1, vertices); // Above
				Vector3 c = getVertexFromIndex(extendedIndex + 1, vertices); // Below
				Vector3 d = getVertexFromIndex(extendedIndex - (settings->numHeight + 2), vertices); // Left
				Vector3 e = getVertexFromIndex(extendedIndex + (settings->numHeight + 2), vertices); // Right

				// Normals
				Vector3 normal1 = Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(d, a));
				Vector3 normal2 = Vector3CrossProduct(Vector3Subtract(d, a), Vector3Subtract(c, a));
				Vector3 normal3 = Vector3CrossProduct(Vector3Subtract(c, a), Vector3Subtract(e, a));
				Vector3 normal4 = Vector3CrossProduct(Vector3Subtract(e, a), Vector3Subtract(b, a));

				// Add and normalize
				Vector3 normal = Vector3Normalize(normal1 + normal2 + normal3 + normal4);
				addNormalToVertex(normal, index);
			}
		}

		delete[] vertices;
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

	void TerrainElement::update(int targetFPS) {
		if (m_reload.load()) {
			reloadMeshData();
			m_reload.store(false);
		}
		if (m_upload.load()) {
			Upload();
			m_upload.store(false);
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

	void TerrainElement::setModelUploaded(bool* modelUploaded) {
		this->modelUploaded = modelUploaded;
	}

	std::atomic<bool>* TerrainElement::getReloadFlag() {
		return &m_reload;
	}

	std::atomic<bool>* TerrainElement::getUploadFlag() {
		return &m_upload;
	}

	void TerrainElement::setNeighbours(std::array<TerrainElement*, 8> neighbours) {
		m_neighbours = neighbours;
	}
}