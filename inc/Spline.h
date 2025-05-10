#pragma once
#include <raylib.h>
#include <vector>
#include "Drawable.h"
#include "Entity.h"
#include "MouseCollider.h"
#include "Gizmo.h"

// Cubic Bezier Spline
class Spline : Entity<Vector3>, Drawable, MouseCollider {
public:
	Spline(std::vector<Vector3> points);

	void draw(int targetFPS, Camera& camera);
	void draw() {}
	void checkCollision(Ray mouseRay);

	void addSegment(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3);

private:
	struct ActivePoint {
		int index = 0;
		Gizmo gizmo;

		ActivePoint(int index, Gizmo gizmo) : index(index), gizmo(gizmo) {}

		bool operator==(const ActivePoint& other) const {
			return index == other.index;
		}
	};

	std::vector<Vector3> m_points;
	std::vector<ActivePoint> m_activePoints;
	float resolution = 0.01f; // The step between evaluations of the spline
	float thickness = 0.1f; // The thickness of the spline
	float sphereMultiplier = 5.0f; // The size of the sphere on the control points (thckness * sphereMultiplier)

	void drawSegment(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 top);
};