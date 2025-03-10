#pragma once
#include "FileAdapters/FileAdapter.h"
#include <string>

class Saveable {
public:
	Saveable(std::string name);

	virtual void save(FileAdapter& file) const = 0;
	virtual void load(FileAdapter& file) const = 0;

	std::string getName() const;

protected:
	std::string m_name = "";
	std::string m_filename;
};