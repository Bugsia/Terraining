﻿// Terraining.cpp : Defines the entry point for the application.
//

#include "Terraining.h"
#include "ThreadPool.h"
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

int main()
{
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

	Terrain::TerrainManager terrainManager(json.getSubElement("Terrain"));
	terrainManager.setCamera(&character);
	terrainManager.setThreadPool(&pool);

	GuiManager guiManager = GuiManager(true);
	guiManager.addGui(std::make_unique<DebugGui::TerrainDebugGui>("Terrain", terrainManager, guiManager));
	guiManager.addGui(std::make_unique<DebugGui::ManipulableTerrainDebugGui>("Manipulable Terrain", terrainManager, character.getCamera()));

	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_LEFT_ALT)) {
			if (cursorActive) DisableCursor();
			else EnableCursor();
			cursorActive = !cursorActive;
		}
		if (IsKeyPressed(KEY_T)) terrainManager.updateElementPositions();
		
		if(!cursorActive) character.update(settings.targetFps);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(character.getCamera());

		DrawGrid(100, 10.0f);
		terrainManager.draw();

		EndMode3D();

		DrawFPS(10.0f, 10.0f);

		guiManager.draw();

		EndDrawing();

		pool.update(settings.targetFps);
		terrainManager.update(settings.targetFps);
	}

	terrainManager.save(json.getSubElement(terrainManager.getName()));
	character.save(json.getSubElement(character.getName()));
	json.save();
	
	return 0;
}
