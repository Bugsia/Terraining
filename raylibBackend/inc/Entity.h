#pragma once
#include <raylib.h>

template <typename T>
class Entity {
public:
	Entity();
	Entity(T position);

	void move(T change);

	T getPosition() const;

protected:
	T m_position;
};

template <typename T>
Entity<T>::Entity() : m_position(T()) {};

template <typename T>
Entity<T>::Entity(T position) : m_position(position) {};

template <typename T>
void Entity<T>::move(T change) {
	m_position += change;
}

template <typename T>
T Entity<T>::getPosition() const {
	return m_position;
}