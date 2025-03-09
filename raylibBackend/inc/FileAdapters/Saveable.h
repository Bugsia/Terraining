#pragma once
#include "FileAdapters/FileAdapter.h"
#include <string>

class Saveable {
public:
	Saveable(); // Maybe at some point remove this constructor to force the user to provide a name
	Saveable(std::string name);

	virtual void save() const = 0;
	virtual void save(std::string filename) const = 0;

protected:
	std::string m_name = "";
	std::string m_filename;

	virtual void save(FileAdapter& file) const = 0;
};

Saveable::Saveable() { }

Saveable::Saveable(std::string name) : m_name(name) { }