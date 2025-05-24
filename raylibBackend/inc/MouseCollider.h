#pragma once
#include <raylib.h>

class MouseCollider {
public:
	struct mouseCollision {
		float distance = 0.0f; // 0.0f means no collision, -1.0f means not neccesiarly hit but contious interaction with the object by user (so ideally keep hit)
		bool* hit = nullptr; // Set to true by mouseCollisionHandler if this object gets the mouseHit based on a criteria (for example closet hit gets hit)
	};

	virtual ~MouseCollider() = default;

	virtual mouseCollision checkCollision(Ray mouseRay) = 0; // Implementations of this function should not do any heavy calculations, just check if the ray collides with the object. update() func should be used for anything else

protected:
	bool m_hit = false;
};