// Terraining.cpp : Defines the entry point for the application.
//

#include "Terraining.h"
#include "ThreadPool.h"
#include "Spline.h"
#include <chrono>
#include <thread>

constexpr auto SETTINGS_FILE = "data/settings.json";
constexpr int INDENTATION = 4;

struct window_settings {
	int width;
	int height;
	std::string title;
	int targetFps;
	std::vector<unsigned int> flags;
};

window_settings loadWindowSettings(const FileAdapter& windowSettings) {
	std::vector<unsigned int> flags;
	std::vector<std::any> anyFlags = windowSettings.getArray("flags").getValue();
	for (std::any flag : anyFlags) {
		flags.push_back(std::any_cast<int>(flag));
	}

	window_settings settings = {
		std::any_cast<int>(windowSettings.getField("width").getValue()),
		std::any_cast<int>(windowSettings.getField("height").getValue()),
		std::any_cast<std::string>(windowSettings.getField("title").getValue()),
		std::any_cast<int>(windowSettings.getField("target_fps").getValue()),
		flags
	};

	return settings;
}

void handleMouseCollisions(Camera& camera, std::vector<MouseCollider*> objects);

int main() {
	JSONAdapter json(SETTINGS_FILE, INDENTATION);

	window_settings settings = loadWindowSettings(json.getSubElement("window_settings"));
	for (unsigned int flag : settings.flags) {
		SetConfigFlags(flag);
	}
	InitWindow(settings.width, settings.height, settings.title.c_str());
	SetTargetFPS(settings.targetFps); 

	ThreadPool pool(5);
	
	Character character(json.getSubElement("MainCamera"));
	bool cursorActive = true;

	Terrain::ManipulableTerrainManager terrainManager("Terrain", json);
	terrainManager.setCamera(&character);
	terrainManager.setThreadPool(&pool);

	terrainManager.useShader("", "data/shaders/default.frag", 0);

	GuiManager guiManager = GuiManager(true);
	guiManager.addGui(std::make_unique<DebugGui::TerrainDebugGui>("Terrain", terrainManager, guiManager));
	guiManager.addGui(std::make_unique<DebugGui::ManipulableTerrainDebugGui>("Manipulable Terrain", terrainManager, character.getCamera()));

	std::vector<MouseCollider*> mouseColliders;

	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_LEFT_ALT)) {
			if (cursorActive) DisableCursor();
			else EnableCursor();
			cursorActive = !cursorActive;
		}
		if (IsKeyPressed(KEY_T)) terrainManager.recalculateElementPosition();
		
		if(!cursorActive) character.update(settings.targetFps);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(character.getCamera());

		DrawGrid(100, 10.0f);
		terrainManager.draw(character.getCamera());

		EndMode3D();

		DrawFPS(10.0f, 10.0f);

		guiManager.draw(character.getCamera());

		EndDrawing();

		pool.update(settings.targetFps);
		terrainManager.update(settings.targetFps);

		handleMouseCollisions(character.getCamera(), mouseColliders);
	}

	terrainManager.save(json);
	character.save(json);
	json.save();
	
	return 0;
}

void handleMouseCollisions(Camera& camera, std::vector<MouseCollider*> objects) {
	MouseCollider::mouseCollision closestCollision = { 0.0f, nullptr };

	for (MouseCollider* object : objects) {
		MouseCollider::mouseCollision collision = object->checkCollision(GetMouseRay(GetMousePosition(), camera));
		if (collision.distance != 0.0f && (collision.distance < closestCollision.distance || closestCollision.distance == 0.0f)) {
			if (closestCollision.hit) *closestCollision.hit = false;
			closestCollision = collision;
		}
		else if (collision.hit) *collision.hit = false;
	}

	if (closestCollision.hit && closestCollision.distance != 0.0f) {
		*closestCollision.hit = true;
	}
}