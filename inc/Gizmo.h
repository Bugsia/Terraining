#pragma once
#include <raylib.h>
#include "Entity.h"
#include "ModelObject.h"
#include "Drawable.h"
#include "MouseCollider.h"

class Gizmo : Entity<Vector3>, Drawable, ModelObject, MouseCollider {
public:
	Gizmo(Vector3& objPosition);
	Gizmo(Vector3& objPosition, std::string arrowXPath, std::string arrowYPath, std::string arrowZPath);

	void draw();
	void update(int targetFPS, const Camera& camera);
	void checkCollision(Ray mouseRay);

private:
	Vector3& m_objPosition;
	char m_hit = 0; // 0 = no hit, 1 = x hit, 2 = y hit, 3 = z hit
	RayCollision m_mouseCollision = { 0 };
	RayCollision m_prevMouseCollision = { 0 };

	Mesh getMeshFromModel(Model model, int meshId);
};