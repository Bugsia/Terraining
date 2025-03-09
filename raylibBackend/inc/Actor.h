#pragma once
#include "Entity.h"
#include "FileAdapters/Saveable.h"

template <typename T>
class Actor : public Entity<T>, public Saveable {
public:
	virtual void save(FileAdapter& file) const;
};

template <typename T>
void Actor<T>::save(FileAdapter& file) const {
	std::vector<std::any> posVec = { this->m_position.x, this->m_position.y };
	if constexpr (std::same_as<T, Vector3>) posVec.push_back(this->m_position.z);
	file.addArray(FileAdapter::FileArray("position", FileAdapter::FLOAT, posVec));
}