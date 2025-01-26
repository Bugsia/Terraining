#pragma once
#include <raylib.h>
#include "Entity.h"

template <typename T>
class MeshEntity : public Entity<T> {
public:
	virtual ~MeshEntity();
	MeshEntity();
	MeshEntity(T position);
	MeshEntity(T position, Mesh mesh);

	MeshEntity(const MeshEntity& other);

	Mesh getMesh() const;

protected:
	Mesh m_mesh;

private:
	template <typename K>
	K* copyIfValid(K* src, int size);
};

template <typename T>
MeshEntity<T>::~MeshEntity() { }

template <typename T>
MeshEntity<T>::MeshEntity() : m_mesh({ 0 }) {}

template <typename T>
MeshEntity<T>::MeshEntity(T position) : Entity<T>(position), m_mesh({ 0 }) {}

template <typename T>
MeshEntity<T>::MeshEntity(T position, Mesh mesh) : Entity<T>(position), m_mesh(mesh) {}

template <typename T>
MeshEntity<T>::MeshEntity(const MeshEntity& other) : Entity<T>(other.m_position), m_mesh(other.m_mesh) {
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

template <typename T>
template <typename K>
K* MeshEntity<T>::copyIfValid(K* src, int size) {
	if (src == nullptr) return nullptr;
	K* dst = (K*)RL_MALLOC(size * sizeof(K));
	memcpy(dst, src, size * sizeof(K));
	return dst;
}

template <typename T>
Mesh MeshEntity<T>::getMesh() const {
	return m_mesh;
}