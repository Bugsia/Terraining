#pragma once
#include <raylib.h>
#include "Spline.h"
#include "Drawable.h"
#include "Entity.h"
#include "Gui.h"

// class RoadBuilder : public Drawable, public Entity<Vector3>, public Gui {
// public:
// 	RoadBuilder(Vector3 position);
// 
// 	void draw(Camera& camera) override;
// 	void update(int targetFPS) override;
// 	bool render() override;
// 
// private:
// 	Spline spline;
// 
// 	bool m_showSpline = false;
// };