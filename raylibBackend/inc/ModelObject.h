#pragma once
#include <raylib.h>
#include <rlgl.h>
#include <concepts>
#include "Entity.h"
#include "Actor.h"
#include "Drawable.h"

class ModelObject {
public:
	virtual ~ModelObject() = default;
	ModelObject();
	ModelObject(Vector3 position);
	ModelObject(Vector3 position, Model model, float scale, Color tint);

	virtual void draw(Vector3 position);

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