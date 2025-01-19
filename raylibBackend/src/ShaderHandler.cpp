#include "ShaderHandler.h"

ShaderHandler::~ShaderHandler() {
	if(shaderSet) UnloadShader(m_shader);
}

void ShaderHandler::activate() {
	BeginShaderMode(m_shader);
}

void ShaderHandler::deactivate() {
	EndShaderMode();
}

void ShaderHandler::useShader(Shader shader) {
	if(shaderSet) UnloadShader(m_shader);
	m_shader = shader;
	shaderSet = true;
}