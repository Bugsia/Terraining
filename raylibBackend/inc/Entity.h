#pragma once
#include "FileAdapters/FileAdapter.h"
#include "FileAdapters/Saveable.h"
#include <raylib.h>
#include <concepts>
#include <any>

template <typename T>
concept baseType = std::same_as<T, Vector2> || std::same_as<T, Vector3>;

template <baseType T>
class Entity {
public:
	virtual ~Entity() = default;
	Entity();
	Entity(T position);

	void move(T change);

	T getPosition() const;

protected:
	T m_position;
};

template <baseType T>
Entity<T>::Entity() : m_position(T()) {};

template <baseType T>
Entity<T>::Entity(T position) : m_position(position) {};

template <baseType T>
void Entity<T>::move(T change) {
	m_position += change;
}

template <baseType T>
T Entity<T>::getPosition() const {
	return m_position;
}