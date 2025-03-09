#include "FileAdapters/Saveable.h"

Saveable::Saveable(std::string name) : m_name(name) { }

std::string Saveable::getName() const {
	return m_name;
}