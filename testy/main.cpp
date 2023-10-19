#include <SDL2/SDL.h>
#include <stdio.h>
#include "Ball.cpp"

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 940

int main(int argc, char* args[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize SDL2: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Mini Golf Mobile", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    Ball ball(20, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        ball.move();
        ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT);

        SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
        SDL_RenderClear(renderer);

        ball.draw(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
