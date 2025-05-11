#include "Spline.h"
#include <raymath.h>

Spline::Spline(std::vector<Vector3> points) : m_points(points) {
}

void Spline::draw(int targetFPS, Camera& camera) {
	Vector3 dir = Vector3Subtract(camera.target, camera.position);
	Vector3 right = Vector3Normalize(Vector3CrossProduct(camera.up, dir));
	Vector3 top = Vector3Normalize(Vector3CrossProduct(right, dir)); // TODO: Calculation of top is not perfect. When looking in a line with the spline the lines is with its thin side to the camera

	for (int i = 0; i < m_points.size(); i += 4) {
		Vector3 p0 = m_points[i];
		Vector3 p1 = m_points[i + 1];
		Vector3 p2 = m_points[i + 2];
		Vector3 p3 = m_points[i + 3];
		drawSegment(p0, p1, p2, p3, top);

		// Draw Sphere on control point
		DrawSphere(p0, m_thickness * m_sphereMultiplier, RED);
	}
	// Draw Sphere on last control point
	DrawSphere(m_points[m_points.size() - 1], m_thickness * m_sphereMultiplier, RED);

	// Draw active points
	for (ActivePoint& point : m_activePoints) {
		// Draw sphers of control points and lines to them
		if (point.index > 0) {
			drawLine(m_points[point.index - 1], m_points[point.index], top);
			DrawSphere(m_points[point.index - 1], m_thickness * m_sphereMultiplier, RED);
			point.gizmo1.update(targetFPS, camera);
			point.gizmo1.draw();
		}
		if (point.index < m_points.size() - 1) {
			drawLine(m_points[point.index], m_points[point.index + 1], top);
			DrawSphere(m_points[point.index + 1], m_thickness * m_sphereMultiplier, RED);
			point.gizmo2.update(targetFPS, camera);
			point.gizmo2.draw();
		}
	
		point.gizmo0.update(targetFPS, camera);
		point.gizmo0.draw();
	}
}

void Spline::checkCollision(Ray mouseRay) {
	// Check collision with gizmo
	for (ActivePoint& point : m_activePoints) {
		point.gizmo0.checkCollision(mouseRay);
		point.gizmo1.checkCollision(mouseRay);
		point.gizmo2.checkCollision(mouseRay);
	}

	// Check collision with control points
	if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return; // Only check collision when mouse is pressed
	for (int i = 0, toggle = 0; i < m_points.size(); ) {
		Vector3 point = m_points[i];
		RayCollision col = GetRayCollisionSphere(mouseRay, point, m_thickness * m_sphereMultiplier);
		if (col.hit) {
			Vector3* control1 = (i > 0) ? &m_points[i - 1] : nullptr;
			Vector3* control2 = (i < m_points.size() - 1) ? &m_points[i + 1] : nullptr;
			ActivePoint activePoint(i, Gizmo(m_thickness * m_gizmoScale, { &m_points[i], control1, control2 }), Gizmo(m_thickness * m_gizmoScale, { control1 }), Gizmo(m_thickness * m_gizmoScale, { control2 }));
			std::vector<ActivePoint>::iterator it = std::find(m_activePoints.begin(), m_activePoints.end(), activePoint);
			if (it == m_activePoints.end()) {
				m_activePoints.push_back(std::move(activePoint));
			}
			else m_activePoints.erase(it);
		}

		i += toggle ? 1 : 3;
		toggle == !toggle;
	}
}

/*
* Adds a segment to the end. The first point is the last point of the last segment
*/
void Spline::addSegment(Vector3 p1, Vector3 p2, Vector3 p3) {
	m_points.push_back(p1);
	m_points.push_back(p2);
	m_points.push_back(p3);
}

void Spline::drawSegment(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 top) {
	// Draw the spline segment using line strips
	Vector3 prevPoint = p0;
	Vector3* pointsA = (Vector3*)RL_CALLOC(2 / m_resolution + 2, sizeof(Vector3)); // pointsA and pointsB are the same, but with different orientations
	Vector3* pointsB = (Vector3*)RL_CALLOC(2 / m_resolution + 2, sizeof(Vector3));
	Vector3 thicknessVector = Vector3Scale(top, m_thickness / 2);

	int index = 0;
	pointsA[index] = prevPoint - thicknessVector;
	pointsB[index++] = prevPoint + thicknessVector;
	pointsA[index] = prevPoint + thicknessVector;
	pointsB[index++] = prevPoint - thicknessVector;
	for (float t = m_resolution; t <= 1.0f; t += m_resolution) {
		float u = 1.0f - t;
		Vector3 point = Vector3Scale(p0, pow(u, 3)) + Vector3Scale(p1, 3 * pow(u, 2) * t) + Vector3Scale(p2, 3 * u * pow(t, 2)) + Vector3Scale(p3, pow(t, 3));
	
		int factor = 1;

		pointsA[index] = point - Vector3Scale(thicknessVector, factor);
		pointsB[index++] = point + Vector3Scale(thicknessVector, factor);
		pointsA[index] = point + Vector3Scale(thicknessVector, factor);
		pointsB[index++] = point - Vector3Scale(thicknessVector, factor);
		
		prevPoint = point;
	}
	
	DrawTriangleStrip3D(pointsA, 2 / m_resolution, BLUE);
	DrawTriangleStrip3D(pointsB, 2 / m_resolution, BLUE);
}

void Spline::drawLine(Vector3 p0, Vector3 p1, Vector3 top) {
	// Draw the spline segment using line strips
	Vector3 prevPoint = p0;
	Vector3* pointsA = (Vector3*)RL_CALLOC(2 / m_resolution + 2, sizeof(Vector3)); // pointsA and pointsB are the same, but with different orientations
	Vector3* pointsB = (Vector3*)RL_CALLOC(2 / m_resolution + 2, sizeof(Vector3));
	Vector3 thicknessVector = Vector3Scale(top, m_thickness / 2);

	int index = 0;
	pointsA[index] = prevPoint - thicknessVector;
	pointsB[index++] = prevPoint + thicknessVector;
	pointsA[index] = prevPoint + thicknessVector;
	pointsB[index++] = prevPoint - thicknessVector;
	for (float t = m_resolution; t <= 1.0f; t += m_resolution) {
		Vector3 point = Vector3Lerp(p0, p1, t);

		int factor = 1;

		pointsA[index] = point - Vector3Scale(thicknessVector, factor);
		pointsB[index++] = point + Vector3Scale(thicknessVector, factor);
		pointsA[index] = point + Vector3Scale(thicknessVector, factor);
		pointsB[index++] = point - Vector3Scale(thicknessVector, factor);

		prevPoint = point;
	}

	DrawTriangleStrip3D(pointsA, 2 / m_resolution, GREEN);
	DrawTriangleStrip3D(pointsB, 2 / m_resolution, GREEN);
}