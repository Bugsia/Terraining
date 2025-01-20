#include "Character.h"

Character::Character() {
	m_camera = { 0 };
	m_camera.position = { 0.0f, 0.0f, 0.0f };
	this->m_position = m_camera.position;
	m_camera.target = { 1.0f, 0.0f, 0.0f };
	m_camera.up = { 0.0f, 1.0f, 0.0f };
	m_camera.fovy = 60.0f;
	m_camera.projection = CAMERA_PERSPECTIVE;
	m_type = CAMERA_CUSTOM;
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
	Vector3 forward = Vector3Subtract(m_camera.target, m_camera.position);
	Vector3 left = Vector3Normalize(Vector3CrossProduct(m_camera.up, forward));
	float deltaSpeed = GetFrameTime() * speed;
	float deltaSensitivity = GetFrameTime() * sensitivity;

	// Keyboard movement
	if (IsKeyDown(KEY_LEFT_SHIFT)) deltaSpeed *= 2.0f;

	Vector3 change = { 0.0f, 0.0f, 0.0f };
	bool applyChange = false;
	if (IsKeyDown(KEY_W)) {
		change = Vector3Add(change, Vector3Scale(forward, deltaSpeed));
		applyChange = true;
	}
	if (IsKeyDown(KEY_S)) {
		change = Vector3Subtract(change, Vector3Scale(forward, deltaSpeed));
		applyChange = true;
	}
	if (IsKeyDown(KEY_A)) {
		change = Vector3Add(change, Vector3Scale(left, deltaSpeed));
		applyChange = true;
	}
	if (IsKeyDown(KEY_D)) {
		change = Vector3Subtract(change, Vector3Scale(left, deltaSpeed));
		applyChange = true;
	}
	if (IsKeyDown(KEY_SPACE)) {
		change = Vector3Add(change, Vector3Scale(m_camera.up, deltaSpeed));
		applyChange = true;
	}
	if (IsKeyDown(KEY_LEFT_CONTROL)) {
		change = Vector3Subtract(change, Vector3Scale(m_camera.up, deltaSpeed));
		applyChange = true;
	}

	if (applyChange) {
		m_camera.position = Vector3Add(m_camera.position, change);
		m_camera.target = Vector3Add(m_camera.target, change);
	}
	TraceLog(LOG_INFO, "Caemra position: %f %f %f", m_camera.position.x, m_camera.position.y, m_camera.position.z);
	// Mouse movement
	Vector2 mouseDelta = GetMouseDelta();
	if (mouseDelta.x != 0.0f && mouseDelta.y != 0.0f) {
		vAngle += -asinf(mouseDelta.y * deltaSensitivity);
		hAngle += asinf(mouseDelta.x * deltaSensitivity);
	
		// Update target
		m_camera.target = Vector3Add(m_camera.position, { cosf(vAngle) * cosf(hAngle), sinf(vAngle), cosf(vAngle) * sinf(hAngle) });
	}
}

Camera Character::getCamera() {
	return m_camera;
}

void Character::setPosition(Vector3 position) {
	m_camera.position = position;
	this->m_position = m_camera.position;
}