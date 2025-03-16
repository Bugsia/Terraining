#pragma once
#include "Entity.h"
#include "FileAdapters/Saveable.h"

template <typename T>
class Actor : public Entity<T>, public Saveable {
public:
	Actor(std::string name);
	Actor(std::string name, T position);

	virtual void save(FileAdapter& file) const;
	virtual bool load(const FileAdapter& file);
};

template <typename T>
Actor<T>::Actor(std::string name) : Saveable(name) {
}

template <typename T>
Actor<T>::Actor(std::string name, T position) : Entity<T>(position), Saveable(name) {
}

template <typename T>
void Actor<T>::save(FileAdapter& file) const {
	std::vector<std::any> posVec = { this->m_position.x, this->m_position.y };
	if constexpr (std::same_as<T, Vector3>) posVec.push_back(this->m_position.z);
	file.getArray("position").setValues(FileAdapter::FLOAT, posVec);
}

template <typename T>
bool Actor<T>::load(const FileAdapter& file) {
	FileAdapter::FileArray posArray = file.getArray("position");
	if (posArray.getKey() == "") return false;
	if (posArray.size() != 2 && posArray.size() != 3) return false;
	else {
		try {
			std::vector<std::any> posVec = posArray.getValue();
			this->m_position.x = any_cast<float>(posVec[0]);
			this->m_position.y = any_cast<float>(posVec[1]);
			if constexpr (std::same_as<T, Vector3>) this->m_position.z = any_cast<float>(posVec[2]);
		}
		catch (const std::bad_any_cast& e) {
			TraceLog(LOG_WARNING, "Actor: bad_any_cast in load function: %s", e.what());
			this->m_position.x = 0.0f;
			this->m_position.y = 0.0f;
			if constexpr (std::same_as<T, Vector3>) this->m_position.z = 0.0f;
			return false;
		}
	}
}