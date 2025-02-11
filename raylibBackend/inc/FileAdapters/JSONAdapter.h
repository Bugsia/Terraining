#pragma once
#include <string>
#include <any>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <utility>
#include <raylib.h>
#include <iomanip>
#include "FileAdapter.h"

class JSONAdapter : public FileAdapter {
public:
	struct JSONAdapterHash {
		size_t operator()(const JSONAdapter& field) const {
			return std::hash<std::string>()(field.getKey());
		}
	};

	JSONAdapter(std::string filename, int indentation);
	JSONAdapter(std::string key);
	JSONAdapter(const std::string filename, std::string key, std::ifstream& file, char& curPart);
	JSONAdapter(std::string key, std::string filename, int indentation);

	virtual FileAdapter& getSubElement(std::string key) override;

	void save() const;
	void save(std::string filename) const;
	void load();

	bool operator==(const JSONAdapter& other) const {
		return m_key == other.m_key;
	}

private:
	const int m_indentation;

	std::string readKey(std::ifstream& file, char& curPart);
	std::pair<std::any, FileAdapter::ValueType> readValue(std::ifstream& file, char& curPart);
	FileField readField(std::ifstream& file, std::string key, char& curPart);
	FileArray readArray(std::ifstream& file, std::string key, char& curPart);

	void writeFieldToFile(std::ofstream& file, FileField field, int indentation) const;

	void load(std::ifstream& file, char& curPart);
	void save(std::ofstream& file, int indentation) const;
};