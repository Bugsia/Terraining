#include "FileAdapters/JSONAdapter.h"

JSONAdapter::JSONAdapter(std::string filename, int indentation) : FileAdapter("", filename), m_indentation(indentation) {
	load();
}

JSONAdapter::JSONAdapter(std::string key) : FileAdapter(key), m_indentation(4) {}

JSONAdapter::JSONAdapter(const std::string filename, std::string key, std::ifstream& file, char& curPart) : FileAdapter(key, filename), m_indentation(4) {
	load(file, curPart);
}

JSONAdapter::JSONAdapter(std::string key, std::string filename, int indentation) : FileAdapter(key, filename), m_indentation(4) {}

FileAdapter& JSONAdapter::getSubElement(std::string key) {
	std::unordered_set<std::shared_ptr<FileAdapter>>::iterator subElement = m_subElements.find(std::make_shared<FileAdapter>(key));
	if (subElement == m_subElements.end()) return **m_subElements.insert(std::make_shared<JSONAdapter>(key, m_filename, 4)).first;
	else return **subElement;
}

void JSONAdapter::save(std::string filename) const {
	std::ofstream file(filename);

	if (!file.is_open()) {
		TraceLog(LOG_WARNING, "JSONAdapter: Could not open file %s", filename.c_str());
		return;
	}
	save(file, m_indentation);
	file << "}\n";

	file.close();
}

void JSONAdapter::save(std::ofstream& file, int indentation) const {
	file << "{\n";

	// Write fields
	for (std::unordered_set<std::shared_ptr<FileField>>::iterator it = m_fields.begin(); it != m_fields.end(); it++) {
		writeFieldToFile(file, **it, indentation);
		if (std::next(it) != m_fields.end() || m_arrays.size() != 0 || m_subElements.size() != 0) file << ",";
		file << "\n";
	}

	// Write arrays
	for (std::unordered_set<std::shared_ptr<FileArray>>::iterator it = m_arrays.begin(); it != m_arrays.end(); it++) {
		file << std::string(indentation, ' ') << "\"" << (*it)->getKey() << "\": [";

		std::vector<std::any> values = (*it)->getValue();
		for (std::vector<std::any>::iterator vecIt = values.begin(); vecIt != values.end(); vecIt++) {
			switch ((*it)->getType()) {
			case FileAdapter::ValueType::BOOL:
				file << std::any_cast<bool>(*vecIt);
				break;
			case FileAdapter::ValueType::INT:
				file << std::any_cast<int>(*vecIt);
				break;
			case FileAdapter::ValueType::FLOAT:
				file << std::any_cast<float>(*vecIt);
				break;
			case FileAdapter::ValueType::STRING:
				file << "\"" << std::any_cast<std::string>(*vecIt) << "\"";
				break;
			}
			if (std::next(vecIt) != values.end()) file << ", ";
		}
		file << "]";
		if (std::next(it) != m_arrays.end() || m_subElements.size() != 0) file << ",";
		file << "\n";
	}

	// Write subJsons
	for (std::unordered_set<std::shared_ptr<FileAdapter>>::iterator it = m_subElements.begin(); it != m_subElements.end(); it++) {
		file << std::string(indentation, ' ') << "\"" << (*it)->getKey() << "\": ";
		(*it)->save(file, indentation + m_indentation);
		file << std::string(indentation, ' ') << "}";
		if (std::next(it) != m_subElements.end()) file << ",";
		file << "\n";
	}
}

void JSONAdapter::writeFieldToFile(std::ofstream& file, FileField field, int indentation) const {
	file << std::string(indentation, ' ') << "\"" << field.getKey() << "\": ";
	switch (field.getType()) {
	case FileAdapter::ValueType::BOOL:
		file << std::any_cast<bool>(field.getValue());
		break;
	case FileAdapter::ValueType::INT:
		file << std::any_cast<int>(field.getValue());
		break;
	case FileAdapter::ValueType::FLOAT:
		// Ensure at least one decimal place is shown, but more are shown when the number is more precise
		if (std::floor(std::any_cast<float>(field.getValue())) == std::any_cast<float>(field.getValue())) file << std::fixed << std::setprecision(1) << std::any_cast<float>(field.getValue());
		else file << std::any_cast<float>(field.getValue());
		break;
	case FileAdapter::ValueType::STRING:
		file << "\"" << std::any_cast<std::string>(field.getValue()) << "\"";
		break;
	}
}

void JSONAdapter::save() const {
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
	load(file, curPart);

	file.close();
}

void JSONAdapter::load(std::ifstream& file, char& curPart) {
	if (curPart != '{') {
		TraceLog(LOG_WARNING, "JSONAdapter: File %s is not a valid JSON file", m_filename.c_str());
		return;
	}

	while (!file.eof()) {
		if (curPart == '\"') {
			std::string key = readKey(file, curPart);

			// Skip to start of value
			while (curPart == ' ' || curPart == ':') file.get(curPart);

			// Check if value is an array or subJson
			if (curPart == '[') {
				FileArray array = readArray(file, key, curPart);
				m_arrays.insert(std::make_shared<FileArray>(array));
			}
			else if (curPart == '{') {
				m_subElements.insert(std::make_shared<JSONAdapter>(m_filename, key, file, curPart));
			}
			else {
				FileField field = readField(file, key, curPart);
				m_fields.insert(std::make_shared<FileField>(field));
			}
		}
		if (curPart == '}') { // End of subJson
			file.get(curPart); // Otherwise parent JSON would also enter this if
			break;
		}
		file.get(curPart);
	}
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

std::pair<std::any, FileAdapter::ValueType> JSONAdapter::readValue(std::ifstream& file, char& curPart) {
	FileAdapter::ValueType type = FileAdapter::ValueType::UNKNOWN;
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
			if (type != FileAdapter::ValueType::STRING && !ignoreNextChar) isDone = true;
			continue;
		case '\"':
			if (ignoreNextChar) break;
			if (type == FileAdapter::ValueType::STRING) isDone = true; // End of string since beginning has already been detected
			type = FileAdapter::ValueType::STRING;
			file.get(curPart);
			continue;
		case '.':
			if (ignoreNextChar) break;
			if (type == FileAdapter::ValueType::UNKNOWN) type = FileAdapter::ValueType::FLOAT;
			break;
		case 't':
		case 'f':
			if (ignoreNextChar) break;
			if (type == FileAdapter::ValueType::UNKNOWN) type = FileAdapter::ValueType::BOOL;
			break;
		case ' ':
			if (ignoreNextChar) break;
			if (type != FileAdapter::ValueType::STRING) {
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
	if (type == FileAdapter::ValueType::UNKNOWN) type = FileAdapter::ValueType::INT;

	// Convert value to correct type
	std::any valueAny;

	switch (type) {
	case FileAdapter::ValueType::BOOL:
		valueAny = (value == "true");
		break;
	case FileAdapter::ValueType::INT:
		valueAny = std::stoi(value);
		break;
	case FileAdapter::ValueType::FLOAT:
		valueAny = std::stof(value);
		break;
	case FileAdapter::ValueType::STRING:
		valueAny = value;
		break;
	}

	// Advance file stream to end of current value (so ,)
	while (curPart != ',' && curPart != '}' && curPart != ']') file.get(curPart);

	return std::pair<std::any, FileAdapter::ValueType>(valueAny, type);
}

JSONAdapter::FileField JSONAdapter::readField(std::ifstream& file, std::string key, char& curPart) {
	std::pair<std::any, FileAdapter::ValueType> value = readValue(file, curPart);
	return FileField(key, value.second, value.first);
}

JSONAdapter::FileArray JSONAdapter::readArray(std::ifstream& file, std::string key, char& curPart) {
	std::vector<std::any> values;
	FileAdapter::ValueType type = FileAdapter::ValueType::UNKNOWN;

	// Check for array end
	file.get(curPart);
	while (curPart != ']') {
		std::pair<std::any, FileAdapter::ValueType> value = readValue(file, curPart);
		values.push_back(value.first);
		type = value.second;
		if (curPart == ']' || curPart == '\n' || curPart == '}') break;
		file.get(curPart);
		while (curPart == ' ') file.get(curPart);
	}
	
	return JSONAdapter::FileArray(key, type, values);
}