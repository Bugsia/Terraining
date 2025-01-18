// Terraining.cpp : Defines the entry point for the application.
//

#include "Terraining.h"

using namespace std;

int main()
{
	InitWindow(1280, 720, "Terraining");
	SetTargetFPS(60);

	GuiManager guiManager(true);
	Terrain::terrain_settings settings = Terrain::terrain_settings();
	settings.radius = 100;
	settings.maxNumElements = 100;
	Terrain::TerrainManager terrainManager = Terrain::TerrainManager(settings);
	{
		DebugGui::TerrainDebugGui terrainDebugGuiReal("Terrain", terrainManager, guiManager);
		guiManager.addGui(std::make_unique<DebugGui::TerrainDebugGui>(terrainDebugGuiReal));
	}

	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(RAYWHITE);

		guiManager.draw();

		EndDrawing();
	}
	return 0;
}
