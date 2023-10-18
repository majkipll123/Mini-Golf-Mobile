

// compile with: g++ main.cpp -o Mini_Golf_Mobile -lSDL2
// run with: ./Mini_Golf_Mobile
#include <SDL2/SDL.h>
#include <stdio.h>


#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 940



int main(int argc, char* args[]) {
  SDL_Window* window = NULL;
  SDL_Surface* screenSurface = NULL;
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow(
			    "Mini Golf Mobile ",
			    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			    SCREEN_WIDTH, SCREEN_HEIGHT,
			    SDL_WINDOW_SHOWN
			    );
  if (window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());
    return 1;
  }
  screenSurface = SDL_GetWindowSurface(window);
  SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0xFF, 0));
  SDL_UpdateWindowSurface(window);
  SDL_Delay(2000);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
