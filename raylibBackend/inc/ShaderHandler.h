#pragma once
#include <raylib.h>
#include <vector>
#include "Drawable.h"

class ShaderHandler : public Drawable {
public:
	~ShaderHandler();

	void activate();
	void deactivate();
	void useShader(Shader shader);

private:
	Shader m_shader;
};