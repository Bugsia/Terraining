// Terraining.cpp : Defines the entry point for the application.
//

#include "Terraining.h"
#include "ThreadPool.h"
#include "Gizmo.h"
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

int notmain()
{
	// Initialize window
	const int screenWidth = 800;
	const int screenHeight = 450;
	InitWindow(screenWidth, screenHeight, "Raylib Custom Shader Example");
	SetTargetFPS(60);

	// Define camera
	Camera3D camera = { 0 };
	camera.position = Vector3({ 5.0f, 5.0f, 5.0f });
	camera.target = Vector3({ 0.0f, 0.0f, 0.0f });
	camera.up = Vector3({ 0.0f, 1.0f, 0.0f });
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	// Load model and texture
	Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
	// Texture2D texture = LoadTexture("data/texture.png");  // Make sure this file exists

	// Load custom shader
	// Shader shader = LoadShader("data/shaders/gizmo.vs", "data/shaders/gizmo.fs");

	// Get shader uniform locations
	// int diffuseMapLoc = GetShaderLocation(shader, "texture0");
	// int colDiffuseLoc = GetShaderLocation(shader, "colDiffuse");

	// Set shader uniform values
	// SetShaderValue(shader, diffuseMapLoc, 0, SHADER_UNIFORM_INT);  // 0 corresponds to texture unit 0
	// Color diffuseColor = RED;
	// Vector4 colorNormalized = {
	// 	(float)diffuseColor.r,
	// 	(float)diffuseColor.g,
	// 	(float)diffuseColor.b,
	// 	(float)diffuseColor.a
	// };
	// SetShaderValue(shader, colDiffuseLoc, &colorNormalized, SHADER_UNIFORM_VEC4);

	// Assign texture and shader to model material
	// model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
	// model.materials[0].shader = shader;

	// Main game loop
	while (!WindowShouldClose()) {
		// Update camera
		UpdateCamera(&camera, CAMERA_ORBITAL);

		// Draw
		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);
		DrawModel(model, Vector3Zero(), 1.0f, WHITE);
		DrawGrid(10, 1.0f);
		EndMode3D();

		DrawFPS(10, 10);
		DrawText("Cube with custom shader and diffuse map", 10, 40, 20, BLACK);
		EndDrawing();
	}

	// Cleanup
	UnloadModel(model);
	// UnloadTexture(texture);
	// UnloadShader(shader);
	CloseWindow();

	return 0;
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

	Terrain::ManipulableTerrainManager terrainManager("Terrain", json);
	terrainManager.setCamera(&character);
	terrainManager.setThreadPool(&pool);

	terrainManager.useShader("", "data/shaders/default.frag", 0);

	GuiManager guiManager = GuiManager(true);
	guiManager.addGui(std::make_unique<DebugGui::TerrainDebugGui>("Terrain", terrainManager, guiManager));
	guiManager.addGui(std::make_unique<DebugGui::ManipulableTerrainDebugGui>("Manipulable Terrain", terrainManager, character.getCamera()));

	Gizmo gizmo("data/models/arrowX.obj", "data/models/arrowY.obj", "data/models/arrowZ.obj");

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
		terrainManager.draw();
		gizmo.draw();
		gizmo.checkCollision(GetScreenToWorldRay(GetMousePosition(), character.getCamera()));
		gizmo.update(settings.targetFps, character.getCamera());

		EndMode3D();

		DrawFPS(10.0f, 10.0f);

		guiManager.draw();

		EndDrawing();

		pool.update(settings.targetFps);
		terrainManager.update(settings.targetFps);
	}

	terrainManager.save(json);
	character.save(json);
	json.save();
	
	return 0;
}