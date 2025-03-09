#pragma once
#include <raylib.h>
#include "Entity.h"
#include "Actor.h"

template <typename T, typename K>
concept baseTypeMesh = std::same_as<T, Entity<K>> || std::same_as<T, Actor<K>>;

template <typename baseTypeMesh>
class MeshObject : public baseTypeMesh {
public:
	virtual ~MeshObject();
	MeshObject();
	MeshObject(Vector3 position);
	MeshObject(Vector3 position, Mesh mesh);

	MeshObject(const MeshObject& other);

	Mesh getMesh() const;
	BoundingBox getBoundingBox() const;

protected:
	Mesh m_mesh;
	BoundingBox m_boundingBox;

	void updateBoundingBox();

private:
	template <typename K>
	K* copyIfValid(K* src, int size);
};

template <typename baseTypeMesh>
MeshObject<baseTypeMesh>::~MeshObject() { }

template <typename baseTypeMesh>
MeshObject<baseTypeMesh>::MeshObject() : m_mesh({ 0 }) {}

template <typename baseTypeMesh>
MeshObject<baseTypeMesh>::MeshObject(Vector3 position) : baseTypeMesh(position), m_mesh({ 0 }) {}

template <typename baseTypeMesh>
MeshObject<baseTypeMesh>::MeshObject(Vector3 position, Mesh mesh) : baseTypeMesh(position), m_mesh(mesh) {}

template <typename baseTypeMesh>
MeshObject<baseTypeMesh>::MeshObject(const MeshObject& other) : baseTypeMesh(other.m_position), m_mesh(other.m_mesh) {
	int vertexCount = m_mesh.vertexCount * 3;
	int texCount = m_mesh.vertexCount * 2;
	int triangleCount = m_mesh.triangleCount * 3;
	m_mesh.vertices = copyIfValid(m_mesh.vertices, vertexCount);
	m_mesh.normals = copyIfValid(m_mesh.normals, vertexCount);
	m_mesh.indices = copyIfValid(m_mesh.indices, triangleCount);
	m_mesh.texcoords = copyIfValid(m_mesh.texcoords, texCount);

	m_mesh.vboId = copyIfValid(m_mesh.vboId, 7);

	if (m_mesh.vaoId > 0) {
		m_mesh.vaoId = 0;
		UploadMesh(&m_mesh, false);
	}
}

template <typename baseTypeMesh>
void MeshObject<baseTypeMesh>::updateBoundingBox() {
	m_boundingBox = GetMeshBoundingBox(m_mesh);
}

template <typename baseTypeMesh>
template <typename K>
K* MeshObject<baseTypeMesh>::copyIfValid(K* src, int size) {
	if (src == nullptr) return nullptr;
	K* dst = (K*)RL_MALLOC(size * sizeof(K));
	memcpy(dst, src, size * sizeof(K));
	return dst;
}

template <typename baseTypeMesh>
Mesh MeshObject<baseTypeMesh>::getMesh() const {
	return m_mesh;
}

template <typename baseTypeMesh>
BoundingBox MeshObject<baseTypeMesh>::getBoundingBox() const {
	return m_boundingBox;
}