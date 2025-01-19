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
#define TERRAIN_RADIUS 200.0f
#define TERRAIN_MAX_NUM_ELEMENTS 500
#define TERRAIN_NUM_HEIGHT 20
#define TERRAIN_NUM_WIDTH 20
#define TERRAIN_SPACING 1.f

int main()
{
	InitWindow(1280, 720, "Terraining");
	SetTargetFPS(60);

	Terrain::terrain_settings terrainSettings = { TERRAIN_RADIUS, TERRAIN_MAX_NUM_ELEMENTS, TERRAIN_NUM_WIDTH, TERRAIN_NUM_HEIGHT, TERRAIN_SPACING };
	Terrain::TerrainManager terrainManager(terrainSettings);
	terrainManager.initializeNoise();
	terrainManager.generateDefaultTerrain();
	terrainManager.initializeModel();

	Character character;
	DisableCursor();

	while (!WindowShouldClose()) {
		character.update();

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(character.getCamera());

		DrawGrid(100, 10.0f);
		terrainManager.draw();

		EndMode3D();

		DrawFPS(10.0f, 10.0f);

		EndDrawing();
	}
	return 0;
}
