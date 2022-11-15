#pragma once
#include <SDL.h>
#include <SDL_image.h>

class Game
{
public:
	Game();
	~Game();
	void Initialize();
	void ProcessInput();
	void Setup();
	void Update();
	void Render();
	void Run();
	void Destroy();

	int windowWidth;
	int windowHeight;

private:
	bool isRunning;
	SDL_Window* window;
	SDL_Renderer* renderer;
};

