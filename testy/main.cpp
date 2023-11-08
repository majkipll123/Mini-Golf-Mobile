#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include "Ball.h"

const float FRICTION = 0.95;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 900

enum GameState {
    MENU,
    LEVEL_1,
    PAUSE,  
    FINAL_SCREEN,// New game state for the pause screen
};

// Define the button structure
struct Button {
    SDL_Rect rect;
    SDL_Surface* textSurface;
    SDL_Texture* textTexture;
    bool isHovered;
};

struct Hole {
    int x;
    int y;
    int radius;
};

bool isCollision(int ballX, int ballY, int ballRadius, const Hole& hole) {
    int dx = ballX - hole.x;
    int dy = ballY - hole.y;
    int distance = std::sqrt(dx * dx + dy * dy);

    return distance <= (ballRadius + hole.radius);
}

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

    // Initialize TTF
    TTF_Init();

    // Create a font
    TTF_Font* font = TTF_OpenFont("/home/majkipll123/Documents/Github/Mini-Golf-Mobile/testy/Beautiful Bride.otf", 54);

    // Create the buttons
    const int buttonWidth = 400;
    const int buttonHeight = 75;
    const int screenWidth = SCREEN_WIDTH;
    Button buttons[3];
    for(int i = 0; i < 3; i++) {
        buttons[i].rect = {(screenWidth - buttonWidth)/2, i*100+100, buttonWidth, buttonHeight};
        //name the buttons
        if ( i==0 )
            buttons[0].textSurface = TTF_RenderText_Solid(font, "Graj", {255, 255, 255});
        if ( i==1 )
            buttons[1].textSurface = TTF_RenderText_Solid(font, "Opcje", {255, 255, 255});
        if ( i==2 )
            buttons[2].textSurface = TTF_RenderText_Solid(font, "Wyjdz", {255, 255, 255});
        
        buttons[i].textTexture = SDL_CreateTextureFromSurface(renderer, buttons[i].textSurface);
        buttons[i].isHovered = false;
    }
    Hole hole;
    hole.x = SCREEN_WIDTH/2 ;/* X-coordinate of the hole */;
    hole.y = SCREEN_WIDTH/5+75 ;/* Y-coordinate of the hole */;
    hole.radius = 15 ;

    Ball ball(20, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 50);
    bool isDragging = false;
    
    float prevMouseX = 0;
    float prevMouseY = 0;
    float xAcceleration = 0.0;
    float yAcceleration = 0.0;
    float deltaX = 0.0;
    float deltaY = 0.0;
    float mouseX = 0.0;
    float mouseY = 0.0;

    GameState gameState = MENU;  // Initial game state

    // Create the "Back to Menu" button
    Button pauseMenuButton;
    pauseMenuButton.rect = {SCREEN_WIDTH - buttonWidth - 20, 20, buttonWidth, buttonHeight};
    pauseMenuButton.textSurface = TTF_RenderText_Solid(font, "Pause", {255, 255, 255});
    pauseMenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, pauseMenuButton.textSurface);
    pauseMenuButton.isHovered = false;

    // Create the "Resume" and "Back to Menu" buttons for the pause screen
    Button resumeButton;
    resumeButton.rect = {(screenWidth - buttonWidth)/2, 550, buttonWidth, buttonHeight};
    resumeButton.textSurface = TTF_RenderText_Solid(font, "Resume", {255, 255, 255});
    resumeButton.textTexture = SDL_CreateTextureFromSurface(renderer, resumeButton.textSurface);
    resumeButton.isHovered = false;

    Button mainMenuButton;
    mainMenuButton.rect = {(screenWidth - buttonWidth)/2,  750, buttonWidth, buttonHeight};
    mainMenuButton.textSurface = TTF_RenderText_Solid(font, "Back to Menu", {255, 255, 255});
    mainMenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, mainMenuButton.textSurface);
    mainMenuButton.isHovered = false;

    Button finalMenuButton;
    finalMenuButton.rect = {(screenWidth - buttonWidth)/2, 450, buttonWidth, buttonHeight};
    finalMenuButton.textSurface = TTF_RenderText_Solid(font, "Wp Back to menu.", {255, 255, 255});
    finalMenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, finalMenuButton.textSurface);
    finalMenuButton.isHovered = false;

    bool quit = false;
    
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                SDL_Point point = {x, y};
                for(int i = 0; i < 3; i++) {
                    if (SDL_PointInRect(&point, &buttons[i].rect)) {
                        buttons[i].isHovered = true;
                    } else {
                        buttons[i].isHovered = false;
                    }
                }
                pauseMenuButton.isHovered = SDL_PointInRect(&point, &pauseMenuButton.rect);
                resumeButton.isHovered = SDL_PointInRect(&point, &resumeButton.rect);
                mainMenuButton.isHovered = SDL_PointInRect(&point, &mainMenuButton.rect);
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    isDragging = true;

                    prevMouseX = e.motion.x;
                    prevMouseY = e.motion.y;

                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    for(int i = 0; i < 3; i++) {
                        if (buttons[i].isHovered && gameState == MENU) {
                            // Handle button click
                            if (i == 0) {
                                // Transition to "1st level"
                                gameState = LEVEL_1;
                                // Initialize the game state for level 1 here
                            }
                            if (i == 2) {

                                quit = true;
                            }
                        }
                    }
                    if (pauseMenuButton.isHovered && gameState == LEVEL_1) {
                        // Transition to the PAUSE state
                        gameState = PAUSE;
                    }
                    if (resumeButton.isHovered && gameState == PAUSE) {
                        // Resume the game
                        gameState = LEVEL_1;
                    }
                    if (mainMenuButton.isHovered && (gameState == FINAL_SCREEN || gameState == PAUSE)) {
                        // Transition back to MENU
                        gameState = MENU;
                        // Add any additional logic for transitioning back to MENU here
                    }
                }
            } else if (e.type == SDL_MOUSEBUTTONUP && ball.isready() && gameState == LEVEL_1) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    isDragging = false;
                    mouseX = e.motion.x;
                    mouseY = e.motion.y;
                    deltaX = mouseX - prevMouseX;
                    deltaY = mouseY - prevMouseY;
                    std::cout << deltaX << " deltaX \n";
                    std::cout << deltaY << " deltaY \n";
                    float xVelocity = static_cast<float>(deltaX) / SCREEN_WIDTH;
                    float yVelocity = static_cast<float>(deltaY) / SCREEN_HEIGHT;
                    ball.setAcceleration(xVelocity, yVelocity, 0.0);
                    ball.setVelocity(xVelocity, yVelocity, 0.0);
                    if (xVelocity !=0 && yVelocity !=0 )
                    ball.increaseHitCount();
                    std::cout<<ball.getHitCount()<<"\n";
               
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
        SDL_RenderClear(renderer);

        if (gameState == MENU) {
            for (int i = 0; i < 3; i++) {
                SDL_SetRenderDrawColor(renderer, buttons[i].isHovered ? 255 : 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &buttons[i].rect);
                SDL_RenderCopy(renderer, buttons[i].textTexture, NULL, &buttons[i].rect);
            }
        } 
        else if (gameState == LEVEL_1) {
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT);
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN;
            }
            ball.move();

            // Draw the hole
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);

            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            

            SDL_SetRenderDrawColor(renderer, pauseMenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pauseMenuButton.rect);
            SDL_RenderCopy(renderer, pauseMenuButton.textTexture, NULL, &pauseMenuButton.rect);

        }
        else if (gameState == PAUSE) {
            // Display the pause screen and buttons
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, resumeButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &resumeButton.rect);
            SDL_RenderCopy(renderer, resumeButton.textTexture, NULL, &resumeButton.rect);

            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);
        }
        else if (gameState == FINAL_SCREEN) {
            // Render the final screen
            std::string endText = "Shots: " + std::to_string(ball.getHitCount());
            SDL_Surface* endSurface = TTF_RenderText_Solid(font, endText.c_str(), {255, 255, 255});
            SDL_Texture* endTexture = SDL_CreateTextureFromSurface(renderer, endSurface);
            SDL_Rect endRect = {(SCREEN_WIDTH - endSurface->w) / 2, (SCREEN_HEIGHT - endSurface->h) / 2, endSurface->w, endSurface->h};
            SDL_RenderCopy(renderer, endTexture, NULL, &endRect);
            SDL_FreeSurface(endSurface);
            SDL_DestroyTexture(endTexture);

            // Add a "Back to Menu" button
            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);

            // Handle button click for "Back to Menu"
          
        /*
        if (gameState == LEVEL_1) {
            // Add the "Back to Menu" button in the top-right part of the screen
            SDL_SetRenderDrawColor(renderer, pauseMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &pauseMenuButton.rect);
            SDL_RenderCopy(renderer, pauseMenuButton.textTexture, NULL, &pauseMenuButton.rect);
        }*/
        

        
        }
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    for (int i = 0; i < 3; i++) {
        SDL_FreeSurface(buttons[i].textSurface);
        SDL_DestroyTexture(buttons[i].textTexture);
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
