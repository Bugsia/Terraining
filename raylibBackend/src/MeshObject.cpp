#include "MeshObject.h"

MeshObject::~MeshObject() { }

MeshObject::MeshObject() : m_mesh({ 0 }) {}

MeshObject::MeshObject(Vector3 position) : m_mesh({ 0 }) {}

MeshObject::MeshObject(Vector3 position, Mesh mesh) : m_mesh(mesh) {}

MeshObject::MeshObject(const MeshObject& other) : m_mesh(other.m_mesh) {
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

void MeshObject::updateBoundingBox() {
	m_boundingBox = GetMeshBoundingBox(m_mesh);
}

template <typename K>
K* MeshObject::copyIfValid(K* src, int size) {
	if (src == nullptr) return nullptr;
	K* dst = (K*)RL_MALLOC(size * sizeof(K));
	memcpy(dst, src, size * sizeof(K));
	return dst;
}

Mesh MeshObject::getMesh() const {
	return m_mesh;
}

BoundingBox MeshObject::getBoundingBox() const {
	return m_boundingBox;
}