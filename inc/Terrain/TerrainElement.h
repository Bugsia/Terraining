#pragma once
#include <raylib.h>
#include <raymath.h>
#include <memory>
#include <atomic>
#include <vector>
#include "MeshObject.h"
#include "Noise.h"
#include "ThreadPool.h"
#include "Entity.h"
#include "Character.h"

#define MAX_MESH_VBO 7

namespace Terrain {
	struct terrain_settings {
		// Terrain manager
		float radius; // Radius of the spawn range of terrain elements
		unsigned int maxNumElements; // The maximum number of terrain elements that can spawn
		bool followCamera = false;
		bool updateWithThreadPool = false;
		ThreadPool* threadPool = nullptr;
		Character* camera = nullptr;
		float distToRelocating = 0.0f;

		// Terrain element
		int numWidth; // The number of verticies along the width of the terrain elements
		int numHeight; // The number of verticies along the height of the terrain elements
		float spacing; // The distance between each vertex
	};

	struct PositionIdentifier {
		int x = 0; // The x-Index of the element
		int i = 1; // -1 if element is in left half, 1 if element is in right half of terrain
		int z = 0; // The z-Index of the element
		int n = 1; // -1 if element is in top half, 1 if element is in bottom half of terrain

		PositionIdentifier() = default;
		PositionIdentifier(int x, int i, int z, int n) : x(x), i(i), z(z), n(n) {}
		PositionIdentifier(const PositionIdentifier& other) : x(other.x), i(other.i), z(other.z), n(other.n) {}

		bool operator==(const PositionIdentifier& other) const {
			return x == other.x && i == other.i && z == other.z && n == other.n;
		}
	};

	class TerrainElement : public MeshObject, public Entity<Vector3> {
	public:
		virtual ~TerrainElement();
		TerrainElement(terrain_settings* settings, PositionIdentifier posId);
		TerrainElement(PositionIdentifier posId);
		TerrainElement(const TerrainElement& other);

		int getIdFromPosId(PositionIdentifier posId);
		void initialiseMesh();
		void initialiseElementWithFlatTerrain();
		void initialiseElementWithNoiseTerrain(Noise::noise_settings* noiseSettings);
		void updateNoiseLayers();
		void randomizeTerrain();
		void updateNormals();
		void updatePosition();
		void Upload();
		void Unload();
		void UnloadLayers();
		void reloadMeshData();
		void renewMeshData();
		void update(int targetFPS);

		// GETTER AND SETTER
		unsigned int getId() const;
		PositionIdentifier getPosId() const;
		Mesh& refMesh();
		void setModelUploaded(bool* modelUploaded);
		std::atomic<bool>* getReloadFlag();
		std::atomic<bool>* getUploadFlag();

		bool operator==(const TerrainElement& other) const {
			return id == other.id;
		}

	protected:
		// General
		unsigned int id = 0; // The unique identifier of the terrain element, which is based on its position
		terrain_settings* settings; // The settings of the terrain (owner is Terrain struct)
		PositionIdentifier posId; // Used to store information about the position of a element in the terrain
		std::atomic<bool> m_reload{ false };
		std::atomic<bool> m_upload{ false };

		// Mesh
		bool dynamicMesh = false; // True if the mesh is dynamic, false otherwise
		bool meshUploaded = false; // True if the mesh has been uploaded to the GPU, false otherwise
		bool* modelUploaded; // The modelUploaded flag of the terrain (owner is Terrain struct)

		// Noise
		Noise::noise_settings* noiseSettings; // The noise settings of the terrain
		std::vector<Color*> noiseLayerPixels; // The pixels of the different noise layers

		Vector3 getPositionFromPosId();
		void flatTerrainVertices();
		void flatTerrainTexcoords();
		void flatTerrainNormals();
		void flatTerrainIndices();
		template<typename T>
		void copyVectorToMemory(T*& dst, std::vector<T> src, bool uploaded);
		void initialiseFlatMesh();
		Vector3 getVertexFromIndex(int index);
		void addNormalToVertex(Vector3 normal, int index);
	};
}

namespace std {
	template<>
	struct hash<Terrain::PositionIdentifier> {
		size_t operator()(const Terrain::PositionIdentifier& pos) const noexcept {
			size_t h1 = hash<int>{}(pos.x);
			size_t h2 = hash<int>{}(pos.i);
			size_t h3 = hash<int>{}(pos.z);
			size_t h4 = hash<int>{}(pos.n);

			return ((h1 ^ (h2 << 1)) >> 1) ^
				((h3 ^ (h4 << 1)) >> 1);
		}
	};
}