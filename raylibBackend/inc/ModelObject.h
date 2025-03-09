#pragma once
#include <raylib.h>
#include <rlgl.h>
#include <concepts>
#include "Entity.h"
#include "Actor.h"
#include "Drawable.h"

template <typename T, typename K>
concept baseTypeModel = std::same_as<T, Entity<K>> || std::same_as<T, Actor<K>>;

template <typename baseTypeModel>
class ModelObject : public Drawable, public baseTypeModel {
public:
	virtual ~ModelObject() = default;
	ModelObject();
	ModelObject(Vector3 position);
	ModelObject(Vector3 position, Model model, float scale, Color tint);

	virtual void draw();

	void useShader(std::string vertexShader, std::string fragmentShader, int materialId);
	void removeShader(int materialId);

	bool getDrawWired();
	void setDrawWired(bool drawWired);
	bool getDrawNormals();
	void setDrawNormals(bool drawNormals);
	float getScale();
	void setScale(float scale);
	Color getTint();
	void setTint(Color tint);
	Model getModel();
	BoundingBox getBoundingBox();

protected:
	Model m_model;
	BoundingBox m_boundingBox;
	float m_scale;
	Color m_tint;
	bool m_drawWired;
	bool m_drawNormals;

	void updateBoundingBox();

private:
	void drawNormals();
};

template <typename baseTypeModel>
ModelObject<baseTypeModel>::ModelObject() : m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false), m_drawNormals(false) {}

template <typename baseTypeModel>
ModelObject<baseTypeModel>::ModelObject(Vector3 position) : baseTypeModel(position), m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false), m_drawNormals(false) {}

template <typename baseTypeModel>
ModelObject<baseTypeModel>::ModelObject(Vector3 position, Model model, float scale, Color tint) : baseTypeModel(position), m_model(model), m_scale(scale), m_tint(tint), m_drawWired(false), m_drawNormals(false) {}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::draw() {
	if (this->m_drawNormals) drawNormals();
	if (this->m_drawWired) DrawModelWires(m_model, this->m_position, m_scale, m_tint);
	else DrawModel(m_model, this->m_position, m_scale, m_tint);
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::useShader(std::string vertexShader, std::string fragmentShader, int materialId) {
	m_model.materials[materialId].shader = LoadShader(vertexShader.c_str(), fragmentShader.c_str());
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::removeShader(int materialId) {
	if (m_model.materials[materialId].shader.id != rlGetShaderIdDefault()) {
		UnloadShader(m_model.materials[materialId].shader);
		m_model.materials[materialId].shader = LoadShader(NULL, NULL);
	}
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::drawNormals() {
	for (int j = 0; j < m_model.meshCount; j++) {
		int vertexCount = m_model.meshes[j].vertexCount * 3;
		for (int i = 0; i < vertexCount; i += 3) {
			Vector3 normal = Vector3({ m_model.meshes[j].normals[i], m_model.meshes[j].normals[i + 1], m_model.meshes[j].normals[i + 2] });
			Vector3 vertex = Vector3({ m_model.meshes[j].vertices[i], m_model.meshes[j].vertices[i + 1], m_model.meshes[j].vertices[i + 2] });
			DrawLine3D(vertex, Vector3Add(vertex, normal), RED);
		}
	}
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::updateBoundingBox() {
	m_boundingBox = GetModelBoundingBox(m_model);
}

template <typename baseTypeModel>
bool ModelObject<baseTypeModel>::getDrawWired() {
	return m_drawWired;
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::setDrawWired(bool drawWired) {
	m_drawWired = drawWired;
}

template <typename baseTypeModel>
bool ModelObject<baseTypeModel>::getDrawNormals() {
	return m_drawNormals;
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::setDrawNormals(bool drawNormals) {
	m_drawNormals = drawNormals;
}

template <typename baseTypeModel>
float ModelObject<baseTypeModel>::getScale() {
	return m_scale;
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::setScale(float scale) {
	m_scale = scale;
}

template <typename baseTypeModel>
Color ModelObject<baseTypeModel>::getTint() {
	return m_tint;
}

template <typename baseTypeModel>
void ModelObject<baseTypeModel>::setTint(Color tint) {
	m_tint = tint;
}

template <typename baseTypeModel>
Model ModelObject<baseTypeModel>::getModel() {
	return m_model;
}

template <typename baseTypeModel>
BoundingBox ModelObject<baseTypeModel>::getBoundingBox() {
	return m_boundingBox;
}