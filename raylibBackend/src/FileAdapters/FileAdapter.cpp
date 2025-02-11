#include "FileAdapters/FileAdapter.h"

FileAdapter::FileAdapter(std::string key) : m_key(key) {}

FileAdapter::FileAdapter(std::string key, std::string filename) : m_key(key), m_filename(filename) {}

std::string FileAdapter::getKey() const {
	return m_key;
}

std::string FileAdapter::getFilename() const {
	return m_filename;
}

FileAdapter::FileField& FileAdapter::getField(std::string key) {
	std::unordered_set<std::shared_ptr<FileField>>::iterator field = m_fields.find(std::make_shared<FileField>(key));
	if (field == m_fields.end()) return **m_fields.insert(std::make_shared<FileField>(key)).first;
	else return **field;
}

FileAdapter::FileField FileAdapter::getField(std::string key) const {
	std::unordered_set<std::shared_ptr<FileField>>::iterator field = m_fields.find(std::make_shared<FileField>(key));
	if (field == m_fields.end()) return FileField("");
	else return **field;
}

FileAdapter::FileArray& FileAdapter::getArray(std::string key) {
	std::unordered_set<std::shared_ptr<FileArray>>::iterator array = m_arrays.find(std::make_shared<FileArray>(key));
	if (array == m_arrays.end()) return **m_arrays.insert(std::make_shared<FileArray>(key)).first;
	else return **array;
}

FileAdapter::FileArray FileAdapter::getArray(std::string key) const {
	std::unordered_set<std::shared_ptr<FileArray>>::iterator array = m_arrays.find(std::make_shared<FileArray>(key));
	if (array == m_arrays.end()) return FileArray("");
	else return **array;
}

FileAdapter& FileAdapter::getSubElement(std::string key) {
	std::unordered_set<std::shared_ptr<FileAdapter>>::iterator subElement = m_subElements.find(std::make_shared<FileAdapter>(key));
	if (subElement == m_subElements.end()) return **m_subElements.insert(std::make_shared<FileAdapter>(key, m_filename)).first;
	else return **subElement;
}

FileAdapter FileAdapter::getSubElement(std::string key) const {
	std::unordered_set<std::shared_ptr<FileAdapter>>::iterator subElement = m_subElements.find(std::make_shared<FileAdapter>(key));
	if (subElement == m_subElements.end()) return FileAdapter("");
	else return **subElement;
}

void FileAdapter::addField(FileField field) {
	m_fields.insert(std::make_shared<FileField>(field));
}

void FileAdapter::addArray(FileArray array) {
	m_arrays.insert(std::make_shared<FileArray>(array));
}

void FileAdapter::addSubElement(FileAdapter subElement) {
	m_subElements.insert(std::make_shared<FileAdapter>(subElement));
}

void FileAdapter::clear() {
	m_fields.clear();
	m_arrays.clear();
	m_subElements.clear();
}