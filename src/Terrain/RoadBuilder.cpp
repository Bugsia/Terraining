#include "Terrain/RoadBuilder.h"

RoadBuilder::RoadBuilder(Vector3 position, Terrain::ManipulableTerrainManager& terrain) : Entity<Vector3>(position), Gui("Road Builder"), m_terrain(terrain), m_spline({ position }) {
}

void RoadBuilder::draw(Camera& camera) {
	if (m_showSpline) {
		m_spline.draw(camera);
	}
}

void RoadBuilder::update(int targetFPS) {
	// TODO
}

bool RoadBuilder::render() {
	ImGui::Begin(m_name.c_str(), &m_open);
	defaultStyle();
	
	ImGui::Checkbox("Show Spline", &m_showSpline);
	if (ImGui::Button("Make Road")) makeRoad();

	ImGui::End();
	return m_open;
}

MouseCollider::mouseCollision RoadBuilder::checkCollision(Ray mouseRay) {
	if (m_showSpline) return m_spline.checkCollision(mouseRay);
	else return { 0.0f, nullptr };
}

void RoadBuilder::makeRoad() {
	if (m_spline.getNumSegments() < 1) {
		TraceLog(LOG_WARNING, "RoadBuilder: Cannot make road with less than 1 segment");
		return;
	}

	float resolution = 0.1f;
	float maxHeight = m_terrain.getHeight(m_spline.evaluate(0.0f));
	for (int i = 1; i <= m_spline.getNumSegments() / resolution; i++) {
		float loc = i * resolution;
		Vector3 point = m_spline.evaluate(loc);
		float height = m_terrain.getHeight(point);
		if (height > maxHeight) {
			maxHeight = height;
		}
	}

	// Add manipulations
	float roadResolution = 0.05f;
	float roadWidth = 10.0f;
	for (int i = 0; i <= m_spline.getNumSegments() / roadResolution; i++) {
		float loc = i * roadResolution;
		m_terrain.manipulateTerrain(Terrain::ManipulableTerrainElement::Y, Terrain::ManipulableTerrainElement::CIRCULAR, Terrain::ManipulableTerrainElement::LEVEL, maxHeight, roadWidth, m_spline.evaluate(loc));
	}
}