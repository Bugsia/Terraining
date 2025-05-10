#pragma once
#include <raylib.h>

class MouseCollider {
public:
	virtual void checkCollision(Ray mouseRay) = 0; // Implementations of this function should not do any heavy calculations, just check if the ray collides with the object. update() func should be used for anything else
};