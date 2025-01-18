#pragma once
#include <raylib.h>
#include "Entity.h"

template <typename T>
class MeshEntity : public Entity<T> {
public:
	MeshEntity();
	MeshEntity(T position);
	MeshEntity(T position, Mesh mesh);

	Mesh getMesh();

protected:
	Mesh m_mesh;
};

template <typename T>
MeshEntity<T>::MeshEntity() : m_mesh({ 0 }) {}

template <typename T>
MeshEntity<T>::MeshEntity(T position) : Entity<T>(position), m_mesh({ 0 }) {}

template <typename T>
MeshEntity<T>::MeshEntity(T position, Mesh mesh) : Entity<T>(position), m_mesh(mesh) {}

template <typename T>
Mesh MeshEntity<T>::getMesh() {
	return m_mesh;
}