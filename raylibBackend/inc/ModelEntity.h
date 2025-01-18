#pragma once
#include <raylib.h>
#include "Entity.h"
#include "Drawable.h"

template <typename T>
class ModelEntity : public Entity<T>, public Drawable {
public:
	ModelEntity();
	ModelEntity(T position);
	ModelEntity(T position, Model modell, float scale, Color tint);

	virtual void draw();

	void setDrawWired(bool drawWired);
	void setScale(float scale);
	void setTint(Color tint);
	Model getModel();

protected:
	Model m_model;
	float m_scale;
	Color m_tint;
	bool m_drawWired;
};

template <typename T>
ModelEntity<T>::ModelEntity() : m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false) {}

template <typename T>
ModelEntity<T>::ModelEntity(T position) : Entity<T>(position), m_model({ 0 }), m_scale(1.0f), m_tint(BLACK), m_drawWired(false) {}

template <typename T>
ModelEntity<T>::ModelEntity(T position, Model model, float scale, Color tint) : Entity<T>(position), m_model(model), m_scale(scale), m_tint(tint), m_drawWired(false) {}

template <typename T>
void ModelEntity<T>::draw() {
	if (this->m_drawWired) DrawModelWires(m_model, this->m_position, m_scale, m_tint);
	else DrawModel(m_model, this->m_position, m_scale, m_tint);
}

template <typename T>
void ModelEntity<T>::setDrawWired(bool drawWired) {
	m_drawWired = drawWired;
}

template <typename T>
void ModelEntity<T>::setScale(float scale) {
	m_scale = scale;
}

template <typename T>
void ModelEntity<T>::setTint(Color tint) {
	m_tint = tint;
}

template <typename T>
Model ModelEntity<T>::getModel() {
	return m_model;
}