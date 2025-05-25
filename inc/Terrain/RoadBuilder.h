#pragma once
#include <raylib.h>
#include "Spline.h"
#include "Drawable.h"
#include "Entity.h"
#include "Gui.h"
#include "MouseCollider.h"
#include "Terrain/ManipulableTerrainManager.h"

class RoadBuilder : public Drawable, public Entity<Vector3>, public Gui, public MouseCollider {
public:
	RoadBuilder(Vector3 position, Terrain::ManipulableTerrainManager& terrain);

	void draw(Camera& camera) override;
	void update(int targetFPS) override;
	bool render() override;
	MouseCollider::mouseCollision checkCollision(Ray mouseRay) override;

private:
	Terrain::ManipulableTerrainManager& m_terrain;
	Spline m_spline;

	bool m_showSpline = false;

	void makeRoad();
};