#include "Character.h"

Character::Character() {
	m_camera = { 0 };
	m_camera.position = { 0.0f, 0.0f, 0.0f };
	this->m_position = m_camera.position;
	m_camera.target = { 1.0f, 0.0f, 0.0f };
	m_camera.up = { 0.0f, 1.0f, 0.0f };
	m_camera.fovy = 60.0f;
	m_camera.projection = CAMERA_PERSPECTIVE;
	this->m_type = CAMERA_CUSTOM;
}

Character::Character(Vector3 position, Vector3 target, Vector3 up, float fovy, CameraProjection projection, int type) : m_camera({ position, target, up, fovy, type }), m_type(type) {
	this->m_position = m_camera.position;
}

void Character::update() {
	if (m_type == CAMERA_CUSTOM) handleInput();
	else UpdateCamera(&m_camera, m_type);
	this->m_position = m_camera.position;
}

void Character::handleInput() {
	// Keyboard movement
	Vector3 change = { 0.0f, 0.0f, 0.0f };
	if (IsKeyDown(KEY_W)) {
		change = Vector3Add(change, Vector3Scale(m_camera.target, speed));
	}
	if (IsKeyDown(KEY_S)) {
		change = Vector3Subtract(change, Vector3Scale(m_camera.target, speed));
	}
	if (IsKeyDown(KEY_A)) {
		change = Vector3Subtract(change, Vector3Scale(Vector3Normalize(Vector3CrossProduct(m_camera.target, m_camera.up)), speed));
	}
	if (IsKeyDown(KEY_D)) {
		change = Vector3Add(change, Vector3Scale(Vector3Normalize(Vector3CrossProduct(m_camera.target, m_camera.up)), speed));
	}
	if (IsKeyDown(KEY_SPACE)) {
		change = Vector3Add(change, Vector3Scale(m_camera.up, speed));
	}
	if (IsKeyDown(KEY_LEFT_CONTROL)) {
		change = Vector3Subtract(change, Vector3Scale(m_camera.up, speed));
	}

	if (change.x != 0.0f && change.y != 0.0f && change.z != 0.0f) {
		m_camera.position = Vector3Add(m_camera.position, change);
	}

	// Mouse movement
	Vector2 mouseDelta = GetMouseDelta();
	if (mouseDelta.x != 0 && mouseDelta.y != 0) {
		TraceLog(LOG_INFO, "Mouse delta: %f, %f", mouseDelta.x, mouseDelta.y);
		vAngle += -asinf(mouseDelta.y * sensitivity);
		hAngle += asinf(mouseDelta.x * sensitivity);

		// Update target
		m_camera.target = Vector3Add(m_camera.position, { cosf(vAngle) * cosf(hAngle), sinf(vAngle), cosf(vAngle) * sinf(hAngle) });

		// Update up
		float upVangle = vAngle + 1 / (2 * PI);
		// m_camera.up = Vector3Add(m_camera.position, { cosf(upVangle) * cosf(hAngle), sinf(upVangle), cosf(upVangle) * sinf(hAngle) });
	}
	else {
		// m_camera.up = Vector3Add(m_camera.up, change);
		m_camera.target = Vector3Add(m_camera.target, change);
	}
}

Camera Character::getCamera() {
	return m_camera;
}

void Character::setPosition(Vector3 position) {
	m_camera.position = position;
	this->m_position = m_camera.position;
}