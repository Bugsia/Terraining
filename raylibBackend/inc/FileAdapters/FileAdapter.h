#pragma once
#include <string>
#include <any>
#include <vector>
#include <unordered_set>
#include <memory>

class FileAdapter {
public:
	enum ValueType {
		UNKNOWN,
		BOOL,
		INT,
		FLOAT,
		STRING
	};

	class FileField {
	public:
		virtual ~FileField() = default;
	
		FileField(std::string key) : m_key(key) {}

		FileField(std::string key, ValueType type, std::any value) : m_key(key), m_type(type), m_value(value) {}

		virtual void setValue(ValueType type, std::any value) {
			this->m_type = type;
			this->m_value = value;
		}
		virtual std::string getKey() const {
			return m_key;
		}
		virtual std::any& getValue() {
			return m_value;
		}
		virtual std::any getValue() const {
			return m_value;
		}
		virtual ValueType getType() const {
			return m_type;
		}

		bool operator==(const FileField& other) const {
			return m_key == other.m_key;
		}

	protected:
		std::string m_key;
		std::any m_value;
		ValueType m_type = ValueType::UNKNOWN;
	};

	struct FileFieldHash {
		size_t operator()(const std::shared_ptr<FileField>& field) const {
			return std::hash<std::string>()(field->getKey());
		}
	};

	struct FileFieldEqual {
		bool operator()(const std::shared_ptr<FileField>& field1, const std::shared_ptr<FileField>& field2) const {
			return field1->getKey() == field2->getKey();
		}
	};

	class FileArray {
	public:
		virtual ~FileArray() = default;
	
		FileArray(std::string key) : m_key(key) {}

		FileArray(std::string key, ValueType type) : m_key(key), m_type(type) {}

		FileArray(std::string key, ValueType type, std::vector<std::any> values) : m_key(key), m_type(type), m_values(values) {}

		virtual void addValue(std::any value) {
			m_values.push_back(value);
		}

		virtual void removeValue(int index) {
			m_values.erase(m_values.begin() + index);
		}

		virtual std::string getKey() const {
			return m_key;
		}
		
		virtual std::vector<std::any>& getValue() {
			return m_values;
		}
		
		virtual std::vector<std::any> getValue() const {
			return m_values;
		}

		virtual ValueType getType() const {
			return m_type;
		}

		bool operator==(const FileArray& other) const {
			return m_key == other.getKey();
		}

	protected:
		std::string m_key;
		FileAdapter::ValueType m_type = ValueType::UNKNOWN;
		std::vector<std::any> m_values;
	};

	struct FileArrayHash {
		size_t operator()(const std::shared_ptr<FileArray>& array) const {
			return std::hash<std::string>()(array->getKey());
		}
	};

	struct FileArrayEqual {
		bool operator()(const std::shared_ptr<FileArray>& array1, const std::shared_ptr<FileArray>& array2) const {
			return array1->getKey() == array2->getKey();
		}
	};

	struct FileAdapterHash {
		size_t operator()(const std::shared_ptr<FileAdapter>& adapter) const {
			return std::hash<std::string>()(adapter->getKey());
		}
	};
	
	struct FileAdapterEqual {
		bool operator()(const std::shared_ptr<FileAdapter>& adapter1, const std::shared_ptr<FileAdapter>& adapter2) const {
			return adapter1->getKey() == adapter2->getKey();
		}
	};

	virtual ~FileAdapter() = default;

	FileAdapter(std::string key);
	FileAdapter(std::string key, std::string filename);

	// Get
	virtual std::string getKey() const;
	virtual std::string getFilename() const;
	virtual FileField& getField(std::string key); // Should add field if it does not exist
	virtual FileField getField(std::string key) const;
	virtual FileArray& getArray(std::string key); // Should add array if it does not exist
	virtual FileArray getArray(std::string key) const;
	virtual FileAdapter& getSubElement(std::string key); // Should add sub element if it does not exist
	virtual FileAdapter getSubElement(std::string key) const;

	// Set
	virtual void addField(FileField field);
	virtual void addArray(FileArray array);
	virtual void addSubElement(FileAdapter subElement);
	virtual void clear();

	// Load/Save
	virtual void load() {}
	virtual void save() const {}
	virtual void save(std::string filename) const {}
	virtual void save(std::ofstream& file, int indentation) const {}

	bool operator==(const FileAdapter& other) const {
		return m_key == other.getKey();
	}

protected:
	const std::string m_filename;
	std::string m_key = "";
	std::unordered_set<std::shared_ptr<FileField>, FileFieldHash, FileFieldEqual> m_fields;
	std::unordered_set<std::shared_ptr<FileArray>, FileArrayHash, FileArrayEqual> m_arrays;
	std::unordered_set<std::shared_ptr<FileAdapter>, FileAdapterHash, FileAdapterEqual> m_subElements;
};