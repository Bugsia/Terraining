#include "Terrain/RoadBuilder.h"

RoadBuilder::RoadBuilder(Vector3 position) : Entity<Vector3>(position), Gui("Road Builder"), spline({ position }) {
}

void RoadBuilder::draw(Camera& camera) {
	if (m_showSpline) {
		spline.draw(camera);
	}
}

void RoadBuilder::update(int targetFPS) {
	// TODO
}

bool RoadBuilder::render() {
	ImGui::Begin(m_name.c_str(), &m_open);
	defaultStyle();
	
	ImGui::Checkbox("Show Spline", &m_showSpline);

	ImGui::End();
	return m_open;
}