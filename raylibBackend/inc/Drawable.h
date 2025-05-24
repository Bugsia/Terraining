#pragma once
#include <raylib.h>

class Drawable {
public:
	virtual void draw(Camera& camera) = 0;
};