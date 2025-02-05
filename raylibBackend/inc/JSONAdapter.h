#pragma once
#include <string>
#include <any>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <utility>
#include <raylib.h>

class JSONAdapter {
public:
	enum JSONValueType {
		JSON_NULL,
		JSON_BOOL,
		JSON_INT,
		JSON_FLOAT,
		JSON_STRING
	};

	class JSONField {
	public:
		JSONField(std::string key);
		JSONField(std::string key, JSONValueType type, std::any value);

		void setValue(JSONValueType type, std::any value);
		std::any getValue();
		std::string getKey() const;

		bool operator==(const JSONField& other) const {
			return m_key == other.m_key;
		}

	private:
		std::string m_key;
		JSONValueType m_type;
		std::any m_value;
	};

	class JSONArray {
	public:

	private:
		std::string m_key;
		JSONValueType m_type;
		std::vector<std::any> m_values;
	};

	JSONAdapter(std::string filename);
	JSONAdapter(std::string filename, int indentation);

	// void addValue(std::string key, JSONValueType type, std::any value);
	// void addArray(std::string key, JSONValueType type, std::vector<std::any> values);
	// std::any getValue(std::string key);
	// 
	// void save();
	void load();

private:
	const int m_indentation = 4;
	const std::string m_filename;
	// std::unordered_set<JSONField> m_fields;

	std::string readKey(std::ifstream& file);
	std::pair<std::any, JSONValueType> readValue(std::ifstream& file);
	JSONField readField(std::ifstream& file);
};

namespace std {
	template <>
	struct hash<JSONAdapter::JSONField> {
		size_t operator()(const JSONAdapter::JSONField& field) const {
			return hash<std::string>()(field.getKey()); // Use std::string hash
		}
	};
}