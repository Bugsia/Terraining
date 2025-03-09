#pragma once
#include "FileAdapter.h"
#include <string>

class Saveable {
public:
	virtual void save() const = 0;
	virtual void save(std::string filename) const = 0;

protected:
	std::string m_name;
	std::string m_filename;

	virtual void save(FileAdapter& file) const = 0;
};