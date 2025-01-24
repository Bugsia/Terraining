#pragma once
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include "Entity.h"

class Character : public Entity<Vector3> {
public:
	Character();
	Character(Vector3 position, Vector3 target, Vector3 up, float fovy, CameraProjection projection, int type);

	void update();

	void setPosition(Vector3 position);
	Camera getCamera();

private:
	Camera m_camera;
	int m_type;
	float sensitivity = 0.03f;
	float speed = 10.0f;
	float hAngle = 0.0f;
	float vAngle = 0.0f;

	void handleInput();
};