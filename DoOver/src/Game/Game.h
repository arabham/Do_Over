#ifndef GAME_H
#define GAME_H

#include "../ECS/ECS.h"
#include <SDL2/SDL.h>


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
	int millisecsPreviousFrame = 0;
	SDL_Window* window;
	SDL_Renderer* renderer;

	std::unique_ptr<Registry> registry;
};

#endif