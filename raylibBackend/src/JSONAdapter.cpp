#include "JSONAdapter.h"

// JSONField
JSONAdapter::JSONField::JSONField(std::string key) : m_key(key), m_type(JSON_NULL) {}

JSONAdapter::JSONField::JSONField(std::string key, JSONValueType type, std::any value) : m_key(key), m_type(type), m_value(value) {}

void JSONAdapter::JSONField::setValue(JSONValueType type, std::any value) {
	m_type = type;
	m_value = value;
}

std::any JSONAdapter::JSONField::getValue() const {
	return m_value;
}

std::string JSONAdapter::JSONField::getKey() const {
	return m_key;
}

JSONAdapter::JSONValueType JSONAdapter::JSONField::getType() const {
	return m_type;
}

// JSONArray
// TODO

// JSONAdapter
JSONAdapter::JSONAdapter(std::string filename) : m_filename(filename) {
	load();
}

JSONAdapter::JSONAdapter(std::string filename, int indentation) : m_filename(filename), m_indentation(indentation) {
	load();
}

void JSONAdapter::save(std::string filename) {
	std::ofstream file(filename);

	if (!file.is_open()) {
		TraceLog(LOG_WARNING, "JSONAdapter: Could not open file %s", filename.c_str());
		return;
	}

	file << "{\n";

	for (std::unordered_set<JSONField>::iterator it = m_fields.begin(); it != m_fields.end(); it++) {
		file << std::string(m_indentation, ' ') << "\"" << it->getKey() << "\": ";
		switch (it->getType()) {
		case JSONValueType::JSON_BOOL:
			file << std::any_cast<bool>(it->getValue());
			break;
		case JSONValueType::JSON_INT:
			file << std::any_cast<int>(it->getValue());
			break;
		case JSONValueType::JSON_FLOAT:
			file << std::any_cast<float>(it->getValue());
			break;
		case JSONValueType::JSON_STRING:
			file << "\"" << std::any_cast<std::string>(it->getValue()) << "\"";
			break;
		}

		if (std::next(it) != m_fields.end()) file << ",";
		file << "\n";
	}

	file << "}\n";
	file.close();
}

void JSONAdapter::save() {
	save(m_filename);
}

void JSONAdapter::load() {
	std::ifstream file(m_filename);

	if (!file.is_open()) {
		TraceLog(LOG_WARNING, "JSONAdapter: Could not open file %s", m_filename.c_str());
		return;
	}

	char curPart;
	file.get(curPart);
	if (curPart != '{') {
		TraceLog(LOG_WARNING, "JSONAdapter: File %s is not a valid JSON file", m_filename.c_str());
		return;
	}

	int index = 0;
	while (!file.eof()) {
		if (curPart == '\"') {
			JSONField field = readField(file);
			m_fields.insert(field);
			index++;
		}
		file.get(curPart);
	}

	file.close();
}

void JSONAdapter::addValue(std::string key, JSONValueType type, std::any value) {
	JSONField field(key, type, value);
	m_fields.insert(field);
}

std::any JSONAdapter::getValue(std::string key) {
	std::unordered_set<JSONField>::iterator it = m_fields.find(JSONField(key));
	if (it != m_fields.end()) return it->getValue();
	else return std::any();
}

std::string JSONAdapter::readKey(std::ifstream& file) {
	char curPart;
	file.get(curPart);

	// Read key
	std::string key = "";

	while (curPart != '\"') {
		key += curPart;
		file.get(curPart);
	}

	return key;
}

std::pair<std::any, JSONAdapter::JSONValueType> JSONAdapter::readValue(std::ifstream& file) {
	char curPart;
	file.get(curPart);

	JSONValueType type = JSONValueType::JSON_NULL;
	std::string value = "";

	// Skip to start of value
	while (curPart == ' ' || curPart == ':') file.get(curPart);

	// Read value and identify type
	bool isDone = false;
	bool ignoreNextChar = false;
	while (!isDone) {
		switch (curPart) {
		case '\\':
			ignoreNextChar = true;
			break;
		case '\"':
			if (ignoreNextChar) break;
			if (type == JSONValueType::JSON_STRING) isDone = true; // End of string since beginning has already been detected
			type = JSONValueType::JSON_STRING;
			file.get(curPart);
			continue;
		case '.':
			if (ignoreNextChar) break;
			if (type == JSONValueType::JSON_NULL) type = JSONValueType::JSON_FLOAT;
			break;
		case 't':
		case 'f':
			if (ignoreNextChar) break;
			if (type == JSONValueType::JSON_NULL) type = JSONValueType::JSON_BOOL;
			break;
		case ' ':
			if (ignoreNextChar) break;
			if (type != JSONValueType::JSON_STRING) {
				isDone = true;
				continue;
			}
			break;
		case '}':
		case '\n':
			isDone = true;
			continue;
			break;
		}

		value += curPart;
		file.get(curPart);
	}
	if (type == JSONValueType::JSON_NULL) type = JSONValueType::JSON_INT;

	// Convert value to correct type
	std::any valueAny;

	switch (type) {
	case JSONValueType::JSON_BOOL:
		valueAny = (value == "true");
		break;
	case JSONValueType::JSON_INT:
		valueAny = std::stoi(value);
		break;
	case JSONValueType::JSON_FLOAT:
		valueAny = std::stof(value);
		break;
	case JSONValueType::JSON_STRING:
		valueAny = value;
		break;
	}

	// Advance file stream to end of current value (so ,)
	while (curPart != ',' && curPart != '}') file.get(curPart);

	return std::pair<std::any, JSONValueType>(valueAny, type);
}

JSONAdapter::JSONField JSONAdapter::readField(std::ifstream& file) {
	std::string key = readKey(file);
	std::pair<std::any, JSONValueType> value = readValue(file);
	return JSONField(key, value.second, value.first);
}