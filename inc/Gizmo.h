#pragma once
#include <raylib.h>
#include <vector>
#include "Entity.h"
#include "ModelObject.h"
#include "Drawable.h"
#include "MouseCollider.h"

class Gizmo : Entity<Vector3>, Drawable, ModelObject, MouseCollider {
public:
	Gizmo(float scale, std::vector<Vector3*> objPositions);
	Gizmo(float scale, std::string arrowXPath, std::string arrowYPath, std::string arrowZPath, std::vector<Vector3*> objPositions);

	void draw();
	void update(int targetFPS, const Camera& camera);
	void checkCollision(Ray mouseRay);

private:
	std::vector<Vector3*> m_objPositions;
	char m_hit = 0; // 0 = no hit, 1 = x hit, 2 = y hit, 3 = z hit
	RayCollision m_mouseCollision = { 0 };
	RayCollision m_prevMouseCollision = { 0 };

	Mesh getMeshFromModel(Model model, int meshId);
	void addToPositions(Vector3 difference);
};