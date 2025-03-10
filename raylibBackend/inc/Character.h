#pragma once
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include "FileAdapters/FileAdapter.h"
#include "FileAdapters/JSONAdapter.h"
#include "Actor.h"

class Character : public Actor<Vector3> {
public:
	Character(std::string name);
	Character(std::string name, Vector3 position, Vector3 target, Vector3 up, float fovy, CameraProjection projection, int type);

	void update();
	void move(Vector3 change);

	void save(FileAdapter& file) const;
	void load(const FileAdapter& file);

	void setPosition(Vector3 position);
	Camera& getCamera();

private:
	Camera m_camera;
	int m_type;
	float m_sensitivity = 0.03f;
	float m_speed = 10.0f;
	float hAngle = 0.0f;
	float vAngle = 0.0f;

	void handleInput();
	void updateCameraTarget(Vector2 mouseDelta);
};