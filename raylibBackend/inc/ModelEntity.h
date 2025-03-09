#pragma once
#include <raylib.h>
#include "Entity.h"
#include "Drawable.h"

template <typename T>
class ModelEntity : public Entity<T>, public Drawable {
public:
	virtual ~ModelEntity() = default;
	ModelEntity();
	ModelEntity(T position);
	ModelEntity(T position, Model modell, float scale, Color tint);

	virtual void draw();

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

template <typename T>
ModelEntity<T>::ModelEntity() : m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false), m_drawNormals(false) {}

template <typename T>
ModelEntity<T>::ModelEntity(T position) : Entity<T>(position), m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false), m_drawNormals(false) {}

template <typename T>
ModelEntity<T>::ModelEntity(T position, Model model, float scale, Color tint) : Entity<T>(position), m_model(model), m_scale(scale), m_tint(tint), m_drawWired(false), m_drawNormals(false) {}

template <typename T>
void ModelEntity<T>::draw() {
	if (this->m_drawNormals) drawNormals();
	if (this->m_drawWired) DrawModelWires(m_model, this->m_position, m_scale, m_tint);
	else DrawModel(m_model, this->m_position, m_scale, m_tint);
}

template <typename T>
void ModelEntity<T>::drawNormals() {
	for (int j = 0; j < m_model.meshCount; j++) {
		int vertexCount = m_model.meshes[j].vertexCount * 3;
		for (int i = 0; i < vertexCount; i += 3) {
			Vector3 normal = Vector3({ m_model.meshes[j].normals[i], m_model.meshes[j].normals[i + 1], m_model.meshes[j].normals[i + 2] });
			Vector3 vertex = Vector3({ m_model.meshes[j].vertices[i], m_model.meshes[j].vertices[i + 1], m_model.meshes[j].vertices[i + 2] });
			DrawLine3D(vertex, Vector3Add(vertex, normal), RED);
		}
	}
}

template <typename T>
void ModelEntity<T>::updateBoundingBox() {
	m_boundingBox = GetModelBoundingBox(m_model);
}

template <typename T>
bool ModelEntity<T>::getDrawWired() {
	return m_drawWired;
}

template <typename T>
void ModelEntity<T>::setDrawWired(bool drawWired) {
	m_drawWired = drawWired;
}

template <typename T>
bool ModelEntity<T>::getDrawNormals() {
	return m_drawNormals;
}

template <typename T>
void ModelEntity<T>::setDrawNormals(bool drawNormals) {
	m_drawNormals = drawNormals;
}

template <typename T>
float ModelEntity<T>::getScale() {
	return m_scale;
}

template <typename T>
void ModelEntity<T>::setScale(float scale) {
	m_scale = scale;
}

template <typename T>
Color ModelEntity<T>::getTint() {
	return m_tint;
}

template <typename T>
void ModelEntity<T>::setTint(Color tint) {
	m_tint = tint;
}

template <typename T>
Model ModelEntity<T>::getModel() {
	return m_model;
}

template <typename T>
BoundingBox ModelEntity<T>::getBoundingBox() {
	return m_boundingBox;
}