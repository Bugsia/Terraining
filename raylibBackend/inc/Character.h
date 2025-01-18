#pragma once
#include <raylib.h>
#include "Entity.h"

template <typename T>
class Character : public Entity<T> {
public:
	Character();
	Character(Vector3 position, Vector3 target, Vector3 up, float fovy, CameraProjection projection, int type) : m_camera({ position, target, up, fovy, type }), m_type(type) {}

	void update();

	Camera getCamera();

private:
	Camera m_camera;
	int m_type;
};

template <typename T>
Character<T>::Character() {
	m_camera = { 0 };
	m_camera.position = { 10.0f, 10.0f, 10.0f };
	this->m_position = m_camera.position;
	m_camera.target = { 0.0f, 1.8f, 0.0f };
	m_camera.up = { 0.0f, 1.0f, 0.0f };
	m_camera.fovy = 60.0f;
	m_camera.projection = CAMERA_PERSPECTIVE;
}

template <typename T>
void Character<T>::update() {
	UpdateCamera(&m_camera, m_type);
	this->m_position = m_camera.position;
}

template <typename T>
Camera Character<T>::getCamera() {
	return m_camera;
}