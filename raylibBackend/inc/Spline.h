#pragma once
#include <raylib.h>
#include <vector>
#include "Drawable.h"
#include "Entity.h"
#include "MouseCollider.h"
#include "Gizmo.h"

// Cubic Bezier Spline
class Spline : Entity<Vector3>, Drawable, public MouseCollider {
public:
	Spline(std::vector<Vector3> points);

	void draw(int targetFPS, Camera& camera);
	void draw() {}
	MouseCollider::mouseCollision checkCollision(Ray mouseRay);

	void addSegment(Vector3 p1, Vector3 p2, Vector3 p3);

	Vector3 evaluate(float loc); // Loc from 0.0f to 1.0f * numSegments
	Vector3 evaluateNorm(float loc); // Loc from 0.0f to 1.0f

private:
	struct ActivePoint {
		int index = 0;
		Gizmo gizmo0; // previous control point gizmo
		Gizmo gizmo1; // actual point gizmo
		Gizmo gizmo2; // next control point gizmo

		ActivePoint(int index, Gizmo gizmo0, Gizmo gizmo1, Gizmo gizmo2) : index(index), gizmo0(gizmo0), gizmo1(gizmo1), gizmo2(gizmo2) {}

		bool operator==(const ActivePoint& other) const {
			return index == other.index;
		}
	};

	std::vector<Vector3> m_points;
	std::vector<ActivePoint> m_activePoints;
	float m_resolution = 0.01f; // The step between evaluations of the spline
	float m_thickness = 0.5f; // The thickness of the spline
	float m_sphereMultiplier = 2.0f; // The size of the sphere on the control points (thckness * sphereMultiplier)
	float m_gizmoScale = 5.0f; // The scale of the gizmo relative to the thickness
	bool m_hideSpheres = false;
	bool m_symmetrical = true;

	void calculateSegmentTriangles(Vector3* points[2], int &index, Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 camPos);
	void drawLine(Vector3 po, Vector3 p1, Vector3 camPos);
	Vector3 evaluatePoints(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t);
};