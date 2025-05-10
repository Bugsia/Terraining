#pragma once
#include <raylib.h>
#include <vector>
#include "Drawable.h"
#include "Entity.h"
#include "MouseCollider.h"

// Cubic Bezier Spline
class Spline : Entity<Vector3>, Drawable, MouseCollider {
public:
	Spline(std::vector<Vector3> points);

	void draw(Camera& camera);
	void draw() {}
	void checkCollision(Ray mouseRay);

	void addSegment(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3);

private:
	std::vector<Vector3> m_points;
	float resolution = 0.01f; // The step between evaluations of the spline
	float thickness = 0.1f; // The thickness of the spline

	void drawSegment(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 top);
};