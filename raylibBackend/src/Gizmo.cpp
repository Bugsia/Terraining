#include "Gizmo.h"
#include <raymath.h>

/*
* The first objPosition is also used as the gizmo position
*/
Gizmo::Gizmo(float scale, std::vector<Vector3*> objPositions) : Gizmo(scale, "data/models/arrowXNormalized.obj", "data/models/arrowYNormalized.obj", "data/models/arrowZNormalized.obj", objPositions) {
}

Gizmo::Gizmo(float scale, std::string arrowXPath, std::string arrowYPath, std::string arrowZPath, std::vector<Vector3*> objPositions) : m_objPositions(objPositions) {
	if (m_objPositions.size() > 0 && m_objPositions[0]) m_position = *m_objPositions[0];
	else m_position = Vector3Zero();
	m_scale = scale;
	 
	// Load arrow mesh (TODO: Only load model once for all gizmos)
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

void Gizmo::draw(Camera& camera) {
	ModelObject::draw(m_position);
}

void Gizmo::move(Vector3 change) {
	addToPositions(change);
}

Vector3 Gizmo::update(int targetFPS, const Camera& camera) {
	if (m_objPositions[0]) m_position = *m_objPositions[0]; // In case the position is changed by something else the gizmo will follow
	if (m_hit && m_dirHit) {
		// get unit vector in direction of the hit arrow
		Vector3 unitHitDirection = Vector3Zero();
		switch (m_dirHit) {
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
		Vector3 difference = Vector3Scale(unitHitDirection, distance);
		addToPositions(difference);
		
		return difference;
	}
	
	return Vector3Zero();
}

MouseCollider::mouseCollision Gizmo::checkCollision(Ray mouseRay) {
	if (m_hit && m_dirHit && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		return { -1.0f, &m_hit }; // Dont lose hit while dragging
	}

	mouseRay.position = Vector3Subtract(mouseRay.position, m_position);
	mouseRay.position = Vector3Scale(mouseRay.position, 1.0f / m_scale);
	RayCollision collision = GetRayCollisionBox(mouseRay, m_boundingBox);
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && collision.hit) {
		// Check which arrow was hit
		for (int i = 0; i < m_model.meshCount; i++) {
			collision = GetRayCollisionMesh(mouseRay, m_model.meshes[i], m_model.transform);
			if (collision.hit) {
				m_dirHit = i + 1;
				if (!m_mouseCollision.hit) m_prevMouseCollision = collision;
				else m_prevMouseCollision = m_mouseCollision;
				m_mouseCollision = collision;
				return { collision.distance, &m_hit };
			}
		}
	}

	m_mouseCollision.hit = false;
	m_dirHit = 0;
	m_hit = false;
	return { 0.0f, nullptr };
}

Mesh Gizmo::getMeshFromModel(Model model, int meshId) {
	if (model.meshCount <= meshId) {
		TraceLog(LOG_ERROR, "Gizmo: Model could not be loaded");
		return Mesh();
	}
	return model.meshes[meshId];
}

void Gizmo::addToPositions(Vector3 difference) {
	m_position = Vector3Add(m_position, difference);
	for (Vector3* objPos : m_objPositions) {
		if(objPos) *objPos = Vector3Add(*objPos, difference);
	}
}