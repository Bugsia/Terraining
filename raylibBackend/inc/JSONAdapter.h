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
		std::any getValue() const;
		std::string getKey() const;
		JSONValueType getType() const;

		bool operator==(const JSONField& other) const {
			return m_key == other.m_key;
		}

	private:
		std::string m_key;
		JSONValueType m_type;
		std::any m_value;
	};

	struct JSONFieldHash {
		size_t operator()(const JSONField& field) const {
			return std::hash<std::string>()(field.getKey());
		}
	};

	class JSONArray {
	public:
		JSONArray(std::string key);
		JSONArray(std::string key, JSONValueType type, std::vector<std::any> values);

		void addValue(std::any value);
		void removeValue(int index);
		std::vector<std::any> getValues() const;
		std::string getKey() const;
		JSONValueType getType() const;

		bool operator==(const JSONArray& other) const {
			return m_key == other.m_key;
		}

	private:
		std::string m_key;
		JSONValueType m_type;
		std::vector<std::any> m_values;
	};

	struct JSONArrayHash {
		size_t operator()(const JSONArray& field) const {
			return std::hash<std::string>()(field.getKey());
		}
	};

	struct JSONAdapterHash {
		size_t operator()(const JSONAdapter& field) const {
			return std::hash<std::string>()(field.getKey());
		}
	};

	JSONAdapter(std::string filename, int indentation);
	JSONAdapter(std::string key);

	void addValue(std::string key, JSONValueType type, std::any value);
	void addArray(std::string key, JSONValueType type, std::vector<std::any> values);
	void addSubJSON(std::string key);
	std::any getValue(std::string key) const;
	std::vector<std::any> getArray(std::string key) const;
	JSONAdapter& getSubJSON(std::string key);
	std::string getKey() const;

	void save() const;
	void save(std::string filename) const;
	void save(std::ofstream& file, int indentation) const;
	void load();

	bool operator==(const JSONAdapter& other) const {
		return m_key == other.m_key;
	}

private:
	const int m_indentation;
	const std::string m_filename;
	std::string m_key = "";
	std::unordered_set<JSONField, JSONFieldHash> m_fields;
	std::unordered_set<JSONArray, JSONArrayHash> m_arrays;
	std::unordered_set<JSONAdapter, JSONAdapterHash> m_subJson;

	JSONAdapter(const std::string filename, std::string key, std::ifstream& file, char& curPart);
	JSONAdapter(std::string key, std::string filename, int indentation);

	std::string readKey(std::ifstream& file, char& curPart);
	std::pair<std::any, JSONValueType> readValue(std::ifstream& file, char& curPart);
	JSONField readField(std::ifstream& file, std::string key, char& curPart);
	JSONArray readArray(std::ifstream& file, std::string key, char& curPart);

	void writeFieldToFile(std::ofstream& file, JSONField field, int indentation) const;

	void load(std::ifstream& file, char& curPart);
};