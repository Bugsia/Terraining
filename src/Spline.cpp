#include "Spline.h"
#include <raymath.h>

Spline::Spline(std::vector<Vector3> points) : m_points(points) {
}

void Spline::draw(Camera& camera) {
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
		DrawSphere(p0, thickness * 5.0f, RED);
	}
	// Draw Sphere on last control point
	DrawSphere(m_points[m_points.size() - 1], thickness * 5.0f, RED);
}

void Spline::checkCollision(Ray mouseRay) {
	// Check collision with control points
	
}

void Spline::addSegment(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3) {
	m_points.push_back(p0);
	m_points.push_back(p1);
	m_points.push_back(p2);
	m_points.push_back(p3);
}

void Spline::drawSegment(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 top) {
	// Draw the spline segment using line strips
	Vector3 prevPoint = p0;
	Vector3* pointsA = (Vector3*)RL_CALLOC(2 / resolution + 2, sizeof(Vector3)); // pointsA and pointsB are the same, but with different orientations
	Vector3* pointsB = (Vector3*)RL_CALLOC(2 / resolution + 2, sizeof(Vector3));
	Vector3 thicknessVector = Vector3Scale(top, thickness / 2);

	int index = 0;
	pointsA[index] = prevPoint - thicknessVector;
	pointsB[index++] = prevPoint + thicknessVector;
	pointsA[index] = prevPoint + thicknessVector;
	pointsB[index++] = prevPoint - thicknessVector;
	for (float t = resolution; t <= 1.0f; t += resolution) {
		float u = 1.0f - t;
		Vector3 point = Vector3Scale(p0, pow(u, 3)) + Vector3Scale(p1, 3 * pow(u, 2) * t) + Vector3Scale(p2, 3 * u * pow(t, 2)) + Vector3Scale(p3, pow(t, 3));
	
		int factor = 1;

		pointsA[index] = point - Vector3Scale(thicknessVector, factor);
		pointsB[index++] = point + Vector3Scale(thicknessVector, factor);
		pointsA[index] = point + Vector3Scale(thicknessVector, factor);
		pointsB[index++] = point - Vector3Scale(thicknessVector, factor);
		
		prevPoint = point;
	}
	
	DrawTriangleStrip3D(pointsA, 2 / resolution, BLUE);
	DrawTriangleStrip3D(pointsB, 2 / resolution, BLUE);
}