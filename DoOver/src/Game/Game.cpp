#include "Game.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisonSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>

int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

Game::Game()
{
	isRunning = false;
	isDebug = false;
	registry = std::make_unique<Registry>();
	assetStore = std::make_unique<AssetStore>();
	eventBus = std::make_unique<EventBus>();
	Logger::Log("Game constructor called!");
}

Game::~Game()
{
	Logger::Log("Game destructor called!");
}

void Game::Initialize()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		Logger::Err("Error initializing SDL.");
		return;
	}
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	windowWidth = displayMode.w;
	windowHeight = displayMode.h;
	window = SDL_CreateWindow
	(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_BORDERLESS
	);
	if (!window)
	{
		Logger::Err("Error creating SDL window.");
		return;
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		Logger::Err("Error creating SDL renderer.");
		return;
	}
	// Initialize the camera view with the entire screen area
	camera.x = 0;
	camera.y = 0;
	camera.w = windowWidth;
	camera.h = windowHeight;
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	isRunning = true;
}

void Game::ProcessInput()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
			{
				isRunning = false;
			}
			if (sdlEvent.key.keysym.sym == SDLK_d)
			{
				isDebug = !isDebug;
			}
			eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
			break;
		}
	}
}


void Game::LoadLevel(int level)
{
	// Add the system that need to be processed in our game
	registry->AddSystem<MovementSystem>();
	registry->AddSystem<RenderSystem>();
	registry->AddSystem<AnimationSystem>();
	registry->AddSystem<CollisionSystem>();
	registry->AddSystem<RenderColliderSystem>();
	registry->AddSystem<DamageSystem>();
	registry->AddSystem<KeyboardControlSystem>();
	registry->AddSystem<CameraMovementSystem>();
	registry->AddSystem<ProjectileEmitSystem>();
	registry->AddSystem<ProjectileLifecycleSystem>();

	// Adding assets to the asset store
	assetStore->AddTexture(renderer, "tank-image", "./assets/images/tank-panther-right.png");
	assetStore->AddTexture(renderer, "truck-image", "./assets/images/truck-ford-right.png");
	assetStore->AddTexture(renderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
	assetStore->AddTexture(renderer, "radar-image", "./assets/images/radar.png");
	assetStore->AddTexture(renderer, "tilemap-image", "./assets/tilemaps/jungle.png");
	assetStore->AddTexture(renderer, "bullet-image", "./assets/images/bullet.png");

	
	// Load the tilemap
	int tileSize = 32;
	double tileScale = 4.0;
	int mapNumCols = 25;
	int mapNumRows = 20;

	std::fstream mapFile;
	mapFile.open("./assets/tilemaps/jungle.map");

	for (int y = 0; y < mapNumRows; y++)
	{
		for (int x = 0; x < mapNumCols; x++)
		{
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.AddComponent<TransformComponent>(glm::vec2(x * (tileScale * tileSize), y * (tileScale * tileSize)), glm::vec2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>("tilemap-image", tileSize, tileSize, 0, false, srcRectX, srcRectY);
		}
	}
	mapFile.close();
	mapWidth = mapNumCols * tileSize * tileScale;
	mapHeight = mapNumRows * tileSize * tileScale;

	// Create some entities
	Entity chopper = registry->CreateEntity();
	chopper.AddComponent<TransformComponent>(glm::vec2(10.0, 100.0), glm::vec2(1.0, 1.0), 0.0);
	chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 1);
	chopper.AddComponent<AnimationComponent>(2, 15, true);
	chopper.AddComponent<KeyboardControlledComponent>(glm::vec2(0, -80), glm::vec2(80, 0), glm::vec2(0, 80), glm::vec2(-80, 0));
	chopper.AddComponent<CameraFollowComponent>();
	chopper.AddComponent<HealthComponent>(100);

	Entity radar = registry->CreateEntity();
	radar.AddComponent<TransformComponent>(glm::vec2(windowWidth - 74, 10.0), glm::vec2(1.0, 1.0), 0.0);
	radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 1, true);
	radar.AddComponent<AnimationComponent>(8, 5, true);

	Entity tank = registry->CreateEntity();
	tank.AddComponent<TransformComponent>(glm::vec2(500.0, 10.0), glm::vec2(1.0, 1.0), 0.0);
	tank.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 1);
	tank.AddComponent<BoxColliderComponent>(32, 32);
	tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 5000, 3000, 0, false);
	tank.AddComponent<HealthComponent>(100);

	Entity truck = registry->CreateEntity();
	truck.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0), glm::vec2(1.0, 1.0), 0.0);
	truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 2);
	truck.AddComponent<BoxColliderComponent>(32, 32);
	truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, 100.0), 2000, 5000, 0, false);
	truck.AddComponent<HealthComponent>(100);
}

void Game::Setup()
{
	LoadLevel(1);
}

void Game::Update()
{
    // If we are too fast, waste some time until we reach the MILLISECS_PER_FRAME
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
	    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
        SDL_Delay(timeToWait);
    }

	// The difference in ticks since the last frame, converted to seconds
	double deltaTime = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0;

	// Store the current frame time
	millisecsPreviousFrame = SDL_GetTicks();

	// Reset all event handleers for the current frame
	eventBus->Reset();

	// Perform the subscription of the events for all systems
	registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(eventBus);

	// Update the registry to process the entities that are waiting to be created/deleted
	registry->Update();
	
	// Inkove all the systems that need to update
	registry->GetSystem<MovementSystem>().Update(deltaTime);
	registry->GetSystem<AnimationSystem>().Update();
	registry->GetSystem<CollisionSystem>().Update(eventBus);
	registry->GetSystem<CameraMovementSystem>().Update(camera);
	registry->GetSystem<ProjectileEmitSystem>().Update(registry);
	registry->GetSystem<ProjectileLifecycleSystem>().Update();

}

void Game::Render()
{
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	// Inkove all the systems that need to render
	registry->GetSystem<RenderSystem>().Update(renderer, assetStore, camera);
	if (isDebug)
	{
		registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
	}
	SDL_RenderPresent(renderer);
}

void Game::Run()
{
	Setup();
	while (isRunning)
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Destroy()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
