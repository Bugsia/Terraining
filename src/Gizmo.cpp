#include "Gizmo.h"
#include <raymath.h>

Gizmo::Gizmo(std::string arrowXPath, std::string arrowYPath, std::string arrowZPath) {
	// Load arrow mesh
	Mesh arrowX = getMeshFromModel(LoadModel(arrowXPath.c_str()), 0);
	Mesh arrowY = getMeshFromModel(LoadModel(arrowYPath.c_str()), 0);
	Mesh arrowZ = getMeshFromModel(LoadModel(arrowZPath.c_str()), 0);

	// Construct model
	setMeshCount(3);
	setMaterialCount(3);
	setMesh(0, arrowX);
	setMaterial(0, 0);
	setMesh(1, arrowY);
	setMaterial(1, 1);
	setMesh(2, arrowZ);
	setMaterial(2, 2);

	// Different colors for each axis
	m_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
	m_model.materials[1].maps[MATERIAL_MAP_DIFFUSE].color = BLUE;
	m_model.materials[2].maps[MATERIAL_MAP_DIFFUSE].color = GREEN;

	updateBoundingBox();
}

void Gizmo::draw() {
	ModelObject::draw(m_position);
}

void Gizmo::update(int targetFPS) {
	if (m_hit == 1) {
		m_position.x += 1.0f * GetFrameTime();
	}
	else if (m_hit == 2) {
		m_position.y += 1.0f * GetFrameTime();
	}
	else if (m_hit == 3) {
		m_position.z += 1.0f * GetFrameTime();
	}
}

void Gizmo::checkCollision(Ray mouseRay) {
	mouseRay.position = Vector3Subtract(mouseRay.position, m_position);
	RayCollision collision = GetRayCollisionBox(mouseRay, m_boundingBox);
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && collision.hit) {
		// Check which arrow was hit
		if (GetRayCollisionMesh(mouseRay, m_model.meshes[0], m_model.transform).hit) {
			m_hit = 1;
			return;
		}
		if (GetRayCollisionMesh(mouseRay, m_model.meshes[1], m_model.transform).hit) {
			m_hit = 2;
			return;
		}
		if (GetRayCollisionMesh(mouseRay, m_model.meshes[2], m_model.transform).hit) {
			m_hit = 3;
			return;
		}
	}

	m_hit = 0;
}

Mesh Gizmo::getMeshFromModel(Model model, int meshId) {
	if (model.meshCount <= meshId) {
		TraceLog(LOG_ERROR, "Gizmo: Model could not be loaded");
		return Mesh();
	}
	return model.meshes[meshId];
}