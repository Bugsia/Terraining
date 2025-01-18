#pragma once
#include <string>
#include "Drawable.h"
#include "imgui.h"

class Gui {
public:
	virtual ~Gui() = default;
	Gui();
	Gui(std::string name);

	virtual bool render() = 0;
	bool isOpen();
	void isOpen(bool open);

protected:
	bool m_open;
	std::string m_name;

	void defaultStyle();
};