#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cmath>
#include <iostream>
#include "Ball.h" 
const float FRICTION = 0.95;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 900

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
    bool isDragging = false;
    float prevMouseX = 0;
    float prevMouseY = 0;
    float xAcceleration = 0.0;
    float yAcceleration = 0.0;
    float deltaX = 0.0;
    float deltaY = 0.0;
    float mouseX = 0.0;
    float mouseY = 0.0;
    bool ready;
    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } // tutaj zablokowac uzytkownikowi mozliwosc uderzania kilka razy z rzedu
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    isDragging = true;

                    prevMouseX = e.motion.x;
                    prevMouseY = e.motion.y;

                    
                }

            }
            else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    isDragging = false;
                    mouseX = e.motion.x;
                    mouseY = e.motion.y;
                    deltaX = mouseX - prevMouseX;
                    deltaY = mouseY - prevMouseY;
                    std::cout<<deltaX<<" deltaX \n";
                    std::cout<<deltaY<<" deltaY \n";
                    float xVelocity = static_cast<float>(deltaX) / SCREEN_WIDTH;
                    float yVelocity = static_cast<float>(deltaY) / SCREEN_HEIGHT;
                    ball.setAcceleration(xVelocity, yVelocity, 0.0);
                    ball.setVelocity(xVelocity, yVelocity, 0.0);
                    
                    //std::cout<<deltaX<<" deltaX \n";
                    //std::cout<<deltaY<<" deltaY \n";   
                    //std::cout<<xVelocity<<" xaccel \n";
                    //std::cout<<yVelocity<<" yaccel \n";
                }
            }
            //else if (e.type ){

                
            //}
           
            
        }
        ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT);
        ball.move();
        

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
