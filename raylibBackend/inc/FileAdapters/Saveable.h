#pragma once
#include "FileAdapters/FileAdapter.h"
#include <string>

class Saveable {
public:
	Saveable() = default; // Maybe at some point remove this constructor to force the user to provide a name
	Saveable(std::string name);

	virtual void save() const = 0;
	virtual void save(std::string filename) const = 0;
	virtual void save(FileAdapter& file) const = 0;

	std::string getName() const;

protected:
	std::string m_name = "";
	std::string m_filename;
};