#include "ModelObject.h"
#include <raymath.h>

ModelObject::ModelObject() : m_model({ 0 }), m_scale(1.0f), m_tint(WHITE), m_drawWired(false), m_drawNormals(false) {
	m_model.transform = MatrixIdentity();
}

ModelObject::ModelObject(Vector3 position) : m_model({ 0 }), m_scale(1.0f), m_tint(WHITE), m_drawWired(false), m_drawNormals(false) {
	m_model.transform = MatrixIdentity();
}

ModelObject::ModelObject(Vector3 position, Model model, float scale, Color tint) : m_model(model), m_scale(scale), m_tint(tint), m_drawWired(false), m_drawNormals(false) {
	m_model.transform = MatrixIdentity();
}

void ModelObject::draw(Vector3 position) {
	if (m_drawNormals) drawNormals();
	if (m_drawWired) DrawModelWires(m_model, position, m_scale, m_tint);
	else DrawModel(m_model, position, m_scale, m_tint);
}

void ModelObject::useShader(std::string vertexShader, std::string fragmentShader, int materialId) {
	const char* vs = vertexShader == "" ? NULL : vertexShader.c_str();
	const char* fs = fragmentShader == "" ? NULL : fragmentShader.c_str();

	Shader shader = LoadShader(vs, fs);
	TraceLog(LOG_DEBUG, "Shader loaded with id: %i", shader.id);
	m_model.materials[materialId].shader = shader;
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

/*
* Keeps the old meshes but copies them to a new array of the new size. If the new size is smaller than the old size, the old meshes, that are too many, are unloaded.
*/
void ModelObject::setMeshCount(int meshCount) {
	if (meshCount < 0) {
		TraceLog(LOG_ERROR, "ModelObject: Mesh count is invalid, cannot set mesh count");
		return;
	}
	else if (meshCount < m_model.meshCount) {
		TraceLog(LOG_WARNING, "ModelObject: Mesh count is smaller than current mesh count. Some Meshes may be lost");
	}

	if (m_model.meshCount != meshCount) {
		int oldMeshCount = m_model.meshCount;
		int smallestMeshCount = meshCount < oldMeshCount ? meshCount : oldMeshCount;

		// Meshes Array
		Mesh* oldMeshes = m_model.meshes;
		m_model.meshCount = meshCount;
		m_model.meshes = (Mesh*)RL_CALLOC(m_model.meshCount, sizeof(Mesh));
		for (int i = 0; i < smallestMeshCount; i++) {
			m_model.meshes[i] = oldMeshes[i];
		}
		if (smallestMeshCount < oldMeshCount) {
			for (int i = smallestMeshCount; i < oldMeshCount; i++) {
				UnloadMesh(oldMeshes[i]);
			}
		}
		RL_FREE(oldMeshes);

		// MeshMaterial Array
		int* oldMeshMaterial = m_model.meshMaterial;
		m_model.meshMaterial = (int*)RL_CALLOC(m_model.meshCount, sizeof(int));
		for (int i = 0; i < smallestMeshCount; i++) {
			m_model.meshMaterial[i] = oldMeshMaterial[i];
		}
		RL_FREE(oldMeshMaterial);
	}
}

/*
* Keeps the old materials but copies them to a new array of the new size. If the new size is smaller than the old size, the old materials, that are too many, are unloaded. New materials are set to be default
*/
void ModelObject::setMaterialCount(int materialCount) {
	if (materialCount < 0) {
		TraceLog(LOG_ERROR, "ModelObject: Material count is invalid, cannot set material count");
		return;
	}
	else if (materialCount < m_model.materialCount) {
		TraceLog(LOG_WARNING, "ModelObject: Material count is smaller than current mesh count. Some Materials may be lost");
	}

	if (m_model.materialCount != materialCount) {
		int oldMaterialCount = m_model.materialCount;
		int smallestMaterialCount = materialCount < oldMaterialCount ? materialCount : oldMaterialCount;
		Material* oldMaterials = m_model.materials;
		m_model.materialCount = materialCount;
		m_model.materials = (Material*)RL_CALLOC(m_model.materialCount, sizeof(Material));
		for (int i = 0; i < materialCount; i++) {
			if (i >= smallestMaterialCount) {
				m_model.materials[i] = LoadMaterialDefault();
			}
			else {
				m_model.materials[i] = oldMaterials[i];
			}
		}
		if (smallestMaterialCount < oldMaterialCount) {
			for (int i = smallestMaterialCount; i < oldMaterialCount; i++) {
				UnloadMaterial(oldMaterials[i]);
			}
		}
		RL_FREE(oldMaterials);
	}
}

/*
* Makes a copy of the mesh, uploads it, and sets it to the mesh with the given id. The old mesh is unloaded.
*/
void ModelObject::setMesh(int meshId, Mesh& mesh) {
	if (meshId < 0 || meshId >= m_model.meshCount) {
		TraceLog(LOG_ERROR, "ModelObject: Mesh id %i is out of bounds", meshId);
		return;
	}
	UnloadMesh(m_model.meshes[meshId]);
	m_model.meshes[meshId] = mesh;
	UploadMesh(&m_model.meshes[meshId], false);
}

void ModelObject::setMaterial(int meshId, int materialId) {
	if (meshId < 0 || meshId >= m_model.meshCount) {
		TraceLog(LOG_ERROR, "ModelObject: Mesh id %i is out of bounds", meshId);
		return;
	}
	if (materialId < 0 || materialId >= m_model.materialCount) {
		TraceLog(LOG_ERROR, "ModelObject: Material id %i is out of bounds", materialId);
		return;
	}
	m_model.meshMaterial[meshId] = materialId;
}

/*
* Gives every mesh the default material. Previous materials are lost / unloaded.
*/
void ModelObject::initializeMaterials() {
	// Unload previous materials
	for (int i = 0; i < m_model.materialCount; i++) {
		UnloadMaterial(m_model.materials[i]);
	}
	RL_FREE(m_model.materials);

	// Set default material
	m_model.materialCount = 1;
	m_model.materials = (Material*)RL_CALLOC(m_model.materialCount, sizeof(Material));

	m_model.materials[0] = LoadMaterialDefault();

	// Apply default material to all meshes
	RL_FREE(m_model.meshMaterial);
	m_model.meshMaterial = (int*)RL_CALLOC(m_model.meshCount, sizeof(int));
	for (int i = 0; i < m_model.meshCount; i++) {
		m_model.meshMaterial[i] = 0;
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