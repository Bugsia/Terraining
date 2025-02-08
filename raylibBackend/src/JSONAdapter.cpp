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
JSONAdapter::JSONArray::JSONArray(std::string key) : m_key(key), m_type(JSON_NULL) {}

JSONAdapter::JSONArray::JSONArray(std::string key, JSONValueType type, std::vector<std::any> values) : m_key(key), m_type(type), m_values(values) {}

void JSONAdapter::JSONArray::addValue(std::any value) {
	m_values.push_back(value);
}

void JSONAdapter::JSONArray::removeValue(int index) {
	m_values.erase(m_values.begin() + index);
}

std::vector<std::any> JSONAdapter::JSONArray::getValues() const {
	return m_values;
}

std::string JSONAdapter::JSONArray::getKey() const {
	return m_key;
}

JSONAdapter::JSONValueType JSONAdapter::JSONArray::getType() const {
	return m_type;
}

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

	// Write fields
	for (std::unordered_set<JSONField>::iterator it = m_fields.begin(); it != m_fields.end(); it++) {
		writeField(file, *it);
		if (std::next(it) != m_fields.end() || m_arrays.size() != 0) file << ",";
		file << "\n";
	}

	// Write arrays
	for (std::unordered_set<JSONArray>::iterator it = m_arrays.begin(); it != m_arrays.end(); it++) {
		file << std::string(m_indentation, ' ') << "\"" << it->getKey() << "\": [";

		std::vector<std::any> values = it->getValues();
		for (std::vector<std::any>::iterator vecIt = values.begin(); vecIt != values.end(); vecIt++) {
			switch (it->getType()) {
			case JSONValueType::JSON_BOOL:
				file << std::any_cast<bool>(*vecIt);
				break;
			case JSONValueType::JSON_INT:
				file << std::any_cast<int>(*vecIt);
				break;
			case JSONValueType::JSON_FLOAT:
				file << std::any_cast<float>(*vecIt);
				break;
			case JSONValueType::JSON_STRING:
				file << "\"" << std::any_cast<std::string>(*vecIt) << "\"";
				break;
			}
			if (std::next(vecIt) != values.end()) file << ", ";
		}
		file << "]";
		if (std::next(it) != m_arrays.end()) file << ",";
		file << "\n";
	}

	file << "}\n";
	file.close();
}

void JSONAdapter::writeField(std::ofstream& file, JSONField field) {
	file << std::string(m_indentation, ' ') << "\"" << field.getKey() << "\": ";
	switch (field.getType()) {
	case JSONValueType::JSON_BOOL:
		file << std::any_cast<bool>(field.getValue());
		break;
	case JSONValueType::JSON_INT:
		file << std::any_cast<int>(field.getValue());
		break;
	case JSONValueType::JSON_FLOAT:
		file << std::any_cast<float>(field.getValue());
		break;
	case JSONValueType::JSON_STRING:
		file << "\"" << std::any_cast<std::string>(field.getValue()) << "\"";
		break;
	}
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
			std::string key = readKey(file, curPart);

			// Skip to start of value
			while (curPart == ' ' || curPart == ':') file.get(curPart);

			// Check if value is an array
			if (curPart == '[') {
				JSONArray array = readArray(file, key, curPart);
				m_arrays.insert(array);
				index++;
			}
			else {
				JSONField field = readField(file, key, curPart);
				m_fields.insert(field);
				index++;
			}
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

std::string JSONAdapter::readKey(std::ifstream& file, char& curPart) {
	// Read key
	file.get(curPart);
	std::string key = "";

	while (curPart != '\"') {
		key += curPart;
		file.get(curPart);
	}

	// Skip to end of value
	file.get(curPart);
	while (curPart == ' ') file.get(curPart);

	return key;
}

std::pair<std::any, JSONAdapter::JSONValueType> JSONAdapter::readValue(std::ifstream& file, char& curPart) {
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
		case ',':
			if (type != JSONValueType::JSON_STRING && !ignoreNextChar) isDone = true;
			continue;
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
		case ']':
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
	while (curPart != ',' && curPart != '}' && curPart != ']') file.get(curPart);

	return std::pair<std::any, JSONValueType>(valueAny, type);
}

JSONAdapter::JSONField JSONAdapter::readField(std::ifstream& file, std::string key, char& curPart) {
	std::pair<std::any, JSONValueType> value = readValue(file, curPart);
	return JSONField(key, value.second, value.first);
}

JSONAdapter::JSONArray JSONAdapter::readArray(std::ifstream& file, std::string key, char& curPart) {
	std::vector<std::any> values;
	JSONValueType type = JSON_NULL;

	// Check for array end
	file.get(curPart);
	while (curPart != ']') {
		std::pair<std::any, JSONValueType> value = readValue(file, curPart);
		values.push_back(value.first);
		type = value.second;
		if (curPart == ']' || curPart == '\n' || curPart == '}') break;
		file.get(curPart);
		while (curPart == ' ') file.get(curPart);
	}
	
	return JSONAdapter::JSONArray(key, type, values);
}