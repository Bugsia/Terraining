#pragma once
#include <raylib.h>
#include <raymath.h>
#include <memory>
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
	};

	class TerrainElement : public MeshObject, public Entity<Vector3> {
	public:
		virtual ~TerrainElement();
		TerrainElement(std::shared_ptr<terrain_settings> settings, PositionIdentifier posId);
		TerrainElement(PositionIdentifier posId);
		TerrainElement(const TerrainElement& other);

		int getIdFromPosId(PositionIdentifier posId);
		void initialiseMesh();
		void initialiseElementWithFlatTerrain();
		void initialiseElementWithNoiseTerrain(std::shared_ptr<Noise::noise_settings> noiseSettings);
		void updateNoiseLayers();
		void randomizeTerrain();
		void updateNormals();
		void updatePosition();
		void Upload();
		void Unload();
		void UnloadLayers();
		void reloadMeshData();
		void renewMeshData();
		void update();

		// GETTER AND SETTER
		unsigned int getId() const;
		PositionIdentifier getPosId() const;
		Mesh& refMesh();
		void setModelUploaded(std::shared_ptr<bool> modelUploaded);
		std::atomic<bool>* getReloadFlag();

		bool operator==(const TerrainElement& other) const {
			return id == other.id;
		}

	protected:
		// General
		unsigned int id = 0; // The unique identifier of the terrain element, which is based on its position
		std::shared_ptr<terrain_settings> settings; // The settings of the terrain (owner is Terrain struct)
		// Vector3 m_position = { 0, 0, 0 }; // The position of the bottom left corner (local x and y = 0) of the terrain Element
		PositionIdentifier posId; // Used to store information about the position of a element in the terrain
		std::atomic<bool> m_reload{ false };

		// Mesh
		bool dynamicMesh = false; // True if the mesh is dynamic, false otherwise
		bool meshUploaded = false; // True if the mesh has been uploaded to the GPU, false otherwise
		std::shared_ptr<bool> modelUploaded; // The modelUploaded flag of the terrain (owner is Terrain struct)

		// Noise
		std::shared_ptr<Noise::noise_settings> noiseSettings; // The noise settings of the terrain
		std::vector<Color*> noiseLayerPixels; // The pixels of the different noise layers

		Vector3 getPositionFromPosId();
		float* flatTerrainVertices();
		float* flatTerrainTexcoords();
		float* flatTerrainNormals();
		unsigned short* flatTerrainIndices();
		template<typename T>
		void copyVectorToMemory(T*& dst, std::vector<T> src, bool uploaded);
		void initialiseFlatMesh();
	};
}