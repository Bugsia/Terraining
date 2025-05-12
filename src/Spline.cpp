#include "Spline.h"
#include <raymath.h>

Spline::Spline(std::vector<Vector3> points) : m_points(points) {
}

void Spline::draw(int targetFPS, Camera& camera) {
	int numPoints = static_cast<int>(m_points.size() / 3) * (2 / m_resolution) + 2;
	Vector3* points[2];
	points[0] = (Vector3*)RL_CALLOC(numPoints, sizeof(Vector3));
	points[1] = (Vector3*)RL_CALLOC(numPoints, sizeof(Vector3));

	// Fill initial value
	Vector3 thickness = Vector3Scale(Vector3Normalize(Vector3CrossProduct(Vector3Subtract(m_points[1], m_points[0]), Vector3Subtract(camera.position, m_points[0]))), m_thickness / 2);
	points[0][0] = m_points[0] - thickness;
	points[0][1] = m_points[0] + thickness;
	points[1][0] = m_points[0] + thickness;
	points[1][1] = m_points[0] - thickness;

	int index = 2;
	for (int i = 0; i < m_points.size() - 1; i += 3) {
		Vector3 p0 = m_points[i];
		Vector3 p1 = m_points[i + 1];
		Vector3 p2 = m_points[i + 2];
		Vector3 p3 = m_points[i + 3];
		// drawSegment(p0, p1, p2, p3, camera.position);
		calculateSegmentTriangles(points, index, p0, p1, p2, p3, camera.position);

		// Draw Sphere on control point
		if (!m_hideSpheres) DrawSphere(p0, m_thickness * m_sphereMultiplier, RED);
	}
	DrawTriangleStrip3D(points[0], numPoints, BLUE);
	DrawTriangleStrip3D(points[1], numPoints, BLUE);

	RL_FREE(points[0]);
	RL_FREE(points[1]);

	// Draw Sphere on last control point
	if (!m_hideSpheres) DrawSphere(m_points[m_points.size() - 1], m_thickness * m_sphereMultiplier, RED);

	// Draw active points
	for (ActivePoint& point : m_activePoints) {
		// Draw sphers of control points and lines to them
		if (point.index > 0) {
			drawLine(m_points[point.index - 1], m_points[point.index], camera.position);
			if(!m_hideSpheres) DrawSphere(m_points[point.index - 1], m_thickness * m_sphereMultiplier / 2, RED);
			point.gizmo1.update(targetFPS, camera);
			point.gizmo1.draw();
		}
		if (point.index < m_points.size() - 1) {
			drawLine(m_points[point.index], m_points[point.index + 1], camera.position);
			if (!m_hideSpheres) DrawSphere(m_points[point.index + 1], m_thickness * m_sphereMultiplier / 2, RED);
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
			if (i == m_points.size() - 1 && IsKeyDown(KEY_LEFT_CONTROL)) {
				Vector3 p = m_points[i];
				addSegment(p + Vector3({ 1.0f, 0.0f, -1.0f }), p + Vector3({ 3.0f, 0.0f, 1.0f }), p + Vector3({ 4.0f, 0.0f, 0.0f }));
			}
			else if (IsKeyDown(KEY_LEFT_SHIFT)) {
				m_hideSpheres = !m_hideSpheres;
			}
			else {
				Vector3* control1 = (i > 0) ? &m_points[i - 1] : nullptr;
				Vector3* control2 = (i < m_points.size() - 1) ? &m_points[i + 1] : nullptr;
				ActivePoint activePoint(i, Gizmo(m_thickness * m_gizmoScale, { &m_points[i], control1, control2 }), Gizmo(m_thickness * m_gizmoScale, { control1 }), Gizmo(m_thickness * m_gizmoScale, { control2 }));
				std::vector<ActivePoint>::iterator it = std::find(m_activePoints.begin(), m_activePoints.end(), activePoint);
				if (it == m_activePoints.end()) {
					m_activePoints.push_back(std::move(activePoint));
				}
				else m_activePoints.erase(it);
			}
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

/*
* points[] needs to be already filled with the first two points of the segment
* index is the index of the first free point in points[]
*/
void Spline::calculateSegmentTriangles(Vector3* points[2], int &index, Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 camPos) {
	Vector3 prevPoint = p0;

	int numIterations = 1 / m_resolution;
	for (int i = 1; i <= numIterations; i++) {
		float t = m_resolution * i;
		Vector3 point = evaluate(p0, p1, p2, p3, t);

		Vector3 dir = Vector3Subtract(point, prevPoint);
		Vector3 camDir = Vector3Subtract(camPos, point);
		Vector3 thicknessDir = Vector3CrossProduct(dir, camDir);
		Vector3 thicknessVector = Vector3Scale(Vector3Normalize(thicknessDir), m_thickness / 2);

		points[0][index] = point - thicknessVector;
		points[1][index++] = point + thicknessVector;
		points[0][index] = point + thicknessVector;
		points[1][index++] = point - thicknessVector;

		prevPoint = point;
	}
}

void Spline::drawLine(Vector3 p0, Vector3 p1, Vector3 camPos) {
	// Draw the spline segment using line strips
	Vector3 prevPoint = p0;
	Vector3* pointsA = (Vector3*)RL_CALLOC(2 / m_resolution + 2, sizeof(Vector3)); // pointsA and pointsB are the same, but with different orientations
	Vector3* pointsB = (Vector3*)RL_CALLOC(2 / m_resolution + 2, sizeof(Vector3));
	Vector3 thicknessVector = Vector3Scale(Vector3UnitY, m_thickness / 2);

	int index = 0;
	pointsA[index] = prevPoint - thicknessVector;
	pointsB[index++] = prevPoint + thicknessVector;
	pointsA[index] = prevPoint + thicknessVector;
	pointsB[index++] = prevPoint - thicknessVector;
	for (float t = m_resolution; t <= 1.0f; t += m_resolution) {
		Vector3 point = Vector3Lerp(p0, p1, t);

		Vector3 dir = Vector3Subtract(point, prevPoint);
		Vector3 camDir = Vector3Subtract(camPos, point);
		Vector3 thick = Vector3CrossProduct(dir, camDir);
		thicknessVector = Vector3Scale(Vector3Normalize(thick), m_thickness / 2);

		pointsA[index] = point - thicknessVector;
		pointsB[index++] = point + thicknessVector;
		pointsA[index] = point + thicknessVector;
		pointsB[index++] = point - thicknessVector;

		prevPoint = point;
	}

	DrawTriangleStrip3D(pointsA, 2 / m_resolution, GREEN);
	DrawTriangleStrip3D(pointsB, 2 / m_resolution, GREEN);
}

Vector3 Spline::evaluate(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float t) {
	float u = 1.0f - t;
	return Vector3Scale(p0, pow(u, 3)) + Vector3Scale(p1, 3 * pow(u, 2) * t) + Vector3Scale(p2, 3 * u * pow(t, 2)) + Vector3Scale(p3, pow(t, 3));
}