#include "Character.h"

Character::Character(std::string name) : Actor<Vector3>(name) {
	m_camera = { 0 };
	m_camera.position = { 0.0f, 0.0f, 0.0f };
	this->m_position = m_camera.position;
	m_camera.target = { 1.0f, 0.0f, 0.0f };
	m_camera.up = { 0.0f, 1.0f, 0.0f };
	m_camera.fovy = 60.0f;
	m_camera.projection = CAMERA_PERSPECTIVE;
	m_type = CAMERA_CUSTOM;
}

Character::Character(std::string name, Vector3 position, Vector3 target, Vector3 up, float fovy, CameraProjection projection, int type) : Actor<Vector3>(name), m_camera({ position, target, up, fovy, type }), m_type(type) {
	m_name = name;
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
	float deltaSpeed = GetFrameTime() * m_speed;
	float deltaSensitivity = GetFrameTime() * m_sensitivity;

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

	// Mouse movement
	Vector2 mouseDelta = GetMouseDelta();
	if (mouseDelta.x != 0.0f && mouseDelta.y != 0.0f) {
		vAngle += -asinf(mouseDelta.y * deltaSensitivity);
		hAngle += asinf(mouseDelta.x * deltaSensitivity);
	
		// Update target
		m_camera.target = Vector3Add(m_camera.position, { cosf(vAngle) * cosf(hAngle), sinf(vAngle), cosf(vAngle) * sinf(hAngle) });
	}
}

void Character::move(Vector3 change) {
	m_camera.position = Vector3Add(m_camera.position, change);
	m_camera.target = Vector3Add(m_camera.target, change);
}

void Character::save(FileAdapter& file) const {
	Actor::save(file);
	file.getField("type").setValue(FileAdapter::INT, m_type);
	file.getField("sensitivity").setValue(FileAdapter::FLOAT, m_sensitivity);
	file.getField("speed").setValue(FileAdapter::FLOAT, m_speed);
	file.getField("hAngle").setValue(FileAdapter::FLOAT, hAngle);
	file.getField("vAngle").setValue(FileAdapter::FLOAT, vAngle);
}

void Character::load(const FileAdapter& file) {
	Actor::load(file);
	m_type = any_cast<int>(file.getField("type").getValue());
	m_sensitivity = any_cast<float>(file.getField("sensitivity").getValue());
	m_speed = any_cast<float>(file.getField("speed").getValue());
	hAngle = any_cast<float>(file.getField("hAngle").getValue());
	vAngle = any_cast<float>(file.getField("vAngle").getValue());
}

Camera& Character::getCamera() {
	return m_camera;
}

void Character::setPosition(Vector3 position) {
	Vector3 deltaPosition = Vector3Subtract(position, m_camera.position);
	m_camera.target = Vector3Add(m_camera.target, deltaPosition);
	m_camera.position = position;
	this->m_position = m_camera.position;
}