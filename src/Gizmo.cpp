#include "Gizmo.h"
#include <raymath.h>

Gizmo::Gizmo(Vector3* objPosition) : Gizmo(objPosition, "data/models/arrowX.obj", "data/models/arrowY.obj", "data/models/arrowZ.obj") {
}

Gizmo::Gizmo(Vector3* objPosition, std::string arrowXPath, std::string arrowYPath, std::string arrowZPath) : m_objPosition(objPosition) {
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

void Gizmo::update(int targetFPS, const Camera& camera) {
	if (m_hit) {
		// get unit vector in direction of the hit arrow
		Vector3 unitHitDirection = Vector3Zero();
		switch (m_hit) {
		case 1:
			unitHitDirection = Vector3({ 1.0f, 0.0f, 0.0f });
			break;
		case 2:
			unitHitDirection = Vector3({ 0.0f, 1.0f, 0.0f });
			break;
		case 3:
			unitHitDirection = Vector3({ 0.0f, 0.0f, 1.0f });
			break;
		}

		// get the direction of the selected arrow (in screen space)
		Vector2 origin = GetWorldToScreen(m_position, camera);
		Vector2 tip = GetWorldToScreen(Vector3Add(m_position, unitHitDirection), camera);
		Vector2 dirArrow = Vector2Subtract(tip, origin);

		// Project the mouse position to the arrow direction
		float factor = Vector2DotProduct(GetMouseDelta(), dirArrow) / Vector2DotProduct(dirArrow, dirArrow);
		Vector2 proj = Vector2Scale(dirArrow, factor);
		int sign = factor < 0 ? -1 : 1; // Correct for direction
		float screenDistance = Vector2Length(proj) * sign;
		float distance = screenDistance / Vector2Length(dirArrow);
		// TraceLog(LOG_INFO, "Distance: %f", distance);

		switch (m_hit) {
		case 1:
			m_position.x += distance;
			m_objPosition->x += distance;
			break;
		case 2:
			m_position.y += distance;
			m_objPosition->y += distance;
			break;
		case 3:
			m_position.z += distance;
			m_objPosition->z += distance;
			break;
		}
	}
}

void Gizmo::checkCollision(Ray mouseRay) {
	if (m_hit && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		return; // Dont lose hit while dragging
	}

	mouseRay.position = Vector3Subtract(mouseRay.position, m_position);
	RayCollision collision = GetRayCollisionBox(mouseRay, m_boundingBox);
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && collision.hit) {
		// Check which arrow was hit
		for (int i = 0; i < m_model.meshCount; i++) {
			collision = GetRayCollisionMesh(mouseRay, m_model.meshes[i], m_model.transform);
			if (collision.hit) {
				m_hit = i + 1;
				if (!m_mouseCollision.hit) m_prevMouseCollision = collision;
				else m_prevMouseCollision = m_mouseCollision;
				m_mouseCollision = collision;
				return;
			}
		}
	}

	m_mouseCollision.hit = false;
	m_hit = 0;
}

Mesh Gizmo::getMeshFromModel(Model model, int meshId) {
	if (model.meshCount <= meshId) {
		TraceLog(LOG_ERROR, "Gizmo: Model could not be loaded");
		return Mesh();
	}
	return model.meshes[meshId];
}