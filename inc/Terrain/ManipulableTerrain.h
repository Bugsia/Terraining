#pragma once
#include <raylib.h>
#include "MeshEntity.h"

class ManipulableTerrain : public MeshEntity<Vector3> {
public:
	virtual ~ManipulableTerrain();
};