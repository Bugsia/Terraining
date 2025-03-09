#pragma once
#include "Entity.h"
#include "FileAdapters/Saveable.h"

template <typename T>
class Actor : public Entity<T>, public Saveable {
public:
	virtual void save(FileAdapter& file) const;
	virtual void load(const FileAdapter& file);
};

template <typename T>
void Actor<T>::save(FileAdapter& file) const {
	std::vector<std::any> posVec = { this->m_position.x, this->m_position.y };
	if constexpr (std::same_as<T, Vector3>) posVec.push_back(this->m_position.z);
	file.getArray("position").setValues(FileAdapter::FLOAT, posVec);
}

template <typename T>
void Actor<T>::load(const FileAdapter& file) {
	FileAdapter::FileArray posArray = file.getArray("position");
	if (posArray.getKey() == "") return;
	else {
		std::vector<std::any> posVec = posArray.getValue();
		this->m_position.x = any_cast<float>(posVec[0]);
		this->m_position.y = any_cast<float>(posVec[1]);
		if constexpr (std::same_as<T, Vector3>) this->m_position.z = any_cast<float>(posVec[2]);
	}
}