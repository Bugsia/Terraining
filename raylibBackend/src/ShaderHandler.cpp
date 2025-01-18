#include "ShaderHandler.h"

ShaderHandler::~ShaderHandler() {
	if(m_shader.id > 0) UnloadShader(m_shader);
}

void ShaderHandler::activate() {
	BeginShaderMode(m_shader);
}

void ShaderHandler::deactivate() {
	EndShaderMode();
}

void ShaderHandler::useShader(Shader shader) {
	UnloadShader(m_shader);
	m_shader = shader;
}