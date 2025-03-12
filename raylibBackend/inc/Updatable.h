#pragma once

class Updatable {
public:
	virtual void update(int targetFPS) = 0;
};