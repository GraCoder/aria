#include "imgui_app.h"
#include <stdio.h>

#include <SDL2/SDL.h>

ImGuiApp::ImGuiApp()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return;
	}

}

void ImGuiApp::run()
{
	while (true) {
		
	};
}
