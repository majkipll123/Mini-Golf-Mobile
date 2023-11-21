// Game.h
#ifndef GAME_H
#define GAME_H

#include "Ball.h"

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    // Add necessary functions and members
    // e.g., handleEvents(), update(), render(), initialize(), cleanup(), etc.

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    Ball ball;
    // Add other game-specific entities and variables
};

#endif
