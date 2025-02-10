// Terraining.cpp : Defines the entry point for the application.
//

#include "Terraining.h"

// Window constants
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "Portfolio"
#define TARGET_FPS 60
#define WINDOW_FLAGS { FLAG_WINDOW_RESIZABLE}

// Terrain constants
#define TERRAIN_RADIUS 350.0f
#define TERRAIN_MAX_NUM_ELEMENTS 1500
#define TERRAIN_NUM_HEIGHT 20
#define TERRAIN_NUM_WIDTH 20
#define TERRAIN_SPACING 1.f

int main()
{
	InitWindow(1280, 720, "Terraining");
	SetTargetFPS(60);
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	Terrain::terrain_settings terrainSettings = { TERRAIN_RADIUS, TERRAIN_MAX_NUM_ELEMENTS, TERRAIN_NUM_WIDTH, TERRAIN_NUM_HEIGHT, TERRAIN_SPACING };
	Terrain::TerrainManager terrainManager(terrainSettings);
	terrainManager.initializeNoise();
	terrainManager.generateDefaultTerrain();
	terrainManager.initializeModel();

	Character character;
	character.setPosition(Vector3({ 100.0f, 50.0f, 100.0f }));
	bool cursorActive = true;

	GuiManager guiManager = GuiManager(true);
	guiManager.addGui(std::make_unique<DebugGui::TerrainDebugGui>("Terrain", terrainManager, guiManager));
	guiManager.addGui(std::make_unique<DebugGui::ManipulableTerrainDebugGui>("Manipulable Terrain", terrainManager, character.getCamera()));

	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_LEFT_ALT)) {
			if (cursorActive) DisableCursor();
			else EnableCursor();
			cursorActive = !cursorActive;
		}
		
		if(!cursorActive) character.update();

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(character.getCamera());

		DrawGrid(100, 10.0f);
		terrainManager.draw();

		EndMode3D();

		DrawFPS(10.0f, 10.0f);

		guiManager.draw();

		EndDrawing();
	}
	return 0;
}
