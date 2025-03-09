#pragma once
#include "FileAdapters/FileAdapter.h"
#include "FileAdapters/Saveable.h"
#include <raylib.h>
#include <concepts>
#include <any>

template <typename T>
concept validTypes = std::same_as<T, Vector2> || std::same_as<T, Vector3>;

template <validTypes T>
class Entity : public Saveable {
public:
	virtual ~Entity() = default;
	Entity();
	Entity(T position);

	void move(T change);
	virtual void save(FileAdapter& file) const;

	T getPosition() const;

protected:
	T m_position;
};

template <validTypes T>
Entity<T>::Entity() : m_position(T()) {};

template <validTypes T>
Entity<T>::Entity(T position) : m_position(position) {};

template <validTypes T>
void Entity<T>::move(T change) {
	m_position += change;
}

template <validTypes T>
void Entity<T>::save(FileAdapter& file) const {
	std::vector<std::any> posVec = { m_position.x, m_position.y };
	if constexpr (std::same_as<T, Vector3>) posVec.push_back(m_position.z);
	file.addArray(FileAdapter::FileArray("position", FileAdapter::FLOAT, posVec));
}

template <validTypes T>
T Entity<T>::getPosition() const {
	return m_position;
}