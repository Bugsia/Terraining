#include "ModelObject.h"
#include <raymath.h>

ModelObject::ModelObject() : m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false), m_drawNormals(false) {}

ModelObject::ModelObject(Vector3 position) : m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false), m_drawNormals(false) {}

ModelObject::ModelObject(Vector3 position, Model model, float scale, Color tint) : m_model(model), m_scale(scale), m_tint(tint), m_drawWired(false), m_drawNormals(false) {}

void ModelObject::draw(Vector3 position) {
	if (m_drawNormals) drawNormals();
	if (m_drawWired) DrawModelWires(m_model, position, m_scale, m_tint);
	else DrawModel(m_model, position, m_scale, m_tint);
}

void ModelObject::useShader(std::string vertexShader, std::string fragmentShader, int materialId) {
	m_model.materials[materialId].shader = LoadShader(vertexShader.c_str(), fragmentShader.c_str());
}

void ModelObject::removeShader(int materialId) {
	if (m_model.materials[materialId].shader.id != rlGetShaderIdDefault()) {
		UnloadShader(m_model.materials[materialId].shader);
		m_model.materials[materialId].shader = LoadShader(NULL, NULL);
	}
}

void ModelObject::drawNormals() {
	for (int j = 0; j < m_model.meshCount; j++) {
		int vertexCount = m_model.meshes[j].vertexCount * 3;
		for (int i = 0; i < vertexCount; i += 3) {
			Vector3 normal = Vector3({ m_model.meshes[j].normals[i], m_model.meshes[j].normals[i + 1], m_model.meshes[j].normals[i + 2] });
			Vector3 vertex = Vector3({ m_model.meshes[j].vertices[i], m_model.meshes[j].vertices[i + 1], m_model.meshes[j].vertices[i + 2] });
			DrawLine3D(vertex, Vector3Add(vertex, normal), RED);
		}
	}
}

void ModelObject::updateBoundingBox() {
	m_boundingBox = GetModelBoundingBox(m_model);
}

bool ModelObject::getDrawWired() {
	return m_drawWired;
}

void ModelObject::setDrawWired(bool drawWired) {
	m_drawWired = drawWired;
}

bool ModelObject::getDrawNormals() {
	return m_drawNormals;
}

void ModelObject::setDrawNormals(bool drawNormals) {
	m_drawNormals = drawNormals;
}

float ModelObject::getScale() {
	return m_scale;
}

void ModelObject::setScale(float scale) {
	m_scale = scale;
}

Color ModelObject::getTint() {
	return m_tint;
}

void ModelObject::setTint(Color tint) {
	m_tint = tint;
}

Model ModelObject::getModel() {
	return m_model;
}

BoundingBox ModelObject::getBoundingBox() {
	return m_boundingBox;
}