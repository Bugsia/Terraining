#pragma once
#include <raylib.h>
#include "Entity.h"
#include "ModelObject.h"
#include "Drawable.h"
#include "MouseCollider.h"

class Gizmo : Entity<Vector3>, Drawable, ModelObject, MouseCollider {
public:
	Gizmo(std::string arrowXPath, std::string arrowYPath, std::string arrowZPath);

	void draw();
	void update(int targetFPS);
	void checkCollision(Ray mouseRay);

private:
	char m_hit = 0; // 0 = no hit, 1 = x hit, 2 = y hit, 3 = z hit

	Mesh getMeshFromModel(Model model, int meshId);
};