#include "GraphicsManager.h"

#include "raylib.h"


////////////////////////////////////
// APIs
////////////////////////////////////
GraphicsManager* GraphicsManager::getInstance() {
	if (!sInstance) {
		sInstance = new GraphicsManager();
	}
	return sInstance;
}

void GraphicsManager::destroyInstance() {
	if (sInstance) {
		delete sInstance;
		sInstance = nullptr;
	}
}

void GraphicsManager::run()
{
	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(RAYWHITE);

		DrawText("Hello, raylib!", 190, 200, 30, DARKGRAY);

		DrawCircle(420, 215, 10, SKYBLUE);

		EndDrawing();
	}

	CloseWindow();
}

////////////////////////////////////
// Private
////////////////////////////////////
GraphicsManager* GraphicsManager::sInstance = nullptr;

GraphicsManager::GraphicsManager()
{
	InitWindow(800, 450, "Rose Aura");
	SetTargetFPS(60);
}

