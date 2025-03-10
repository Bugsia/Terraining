#pragma once
#include <raylib.h>
#include "Entity.h"
#include "Actor.h"

class MeshObject {
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