#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include<unistd.h>
#include "Ball.h"
#include "Wall.h"

const float FRICTION = 0.95;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 900

enum GameState {
    MENU,
    CHOOSE_LVL,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
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

Wall::Wall(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height) {}

void Wall::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Set the color to blue (adjust as needed)
    SDL_Rect wallRect = {x, y, width, height};
    SDL_RenderFillRect(renderer, &wallRect);
}

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
        return 0;
    }

    window = SDL_CreateWindow("Mini Golf Mobile", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
        return 0;
    }


    // Initialize TTF
    TTF_Init();

    // Create a font
    TTF_Font* font = TTF_OpenFont("/home/majkipll123/Documents/Github/Mini-Golf-Mobile/testy/Beautiful Bride.otf", 54);

    // Create the buttons
    const int buttonWidth = 400;
    const int buttonHeight = 75;
    const int screenWidth = SCREEN_WIDTH;
    Button buttons[6];
    //Button buttons[3];

    for(int i = 0; i < 5; i++) {
        buttons[i].rect = {(screenWidth - buttonWidth)/2, i*100+100, buttonWidth, buttonHeight};
        //name the buttons
        if ( i==0 )
            buttons[0].textSurface = TTF_RenderText_Solid(font, "Practise offline", {255, 255, 255});
        else if ( i==1 )
            buttons[1].textSurface = TTF_RenderText_Solid(font, "Options", {255, 255, 255});
        else if ( i==2 )
            buttons[2].textSurface = TTF_RenderText_Solid(font, "Quit", {255, 255, 255});
        else if ( i==3 )
            buttons[3].textSurface = TTF_RenderText_Solid(font, "Level one ", {255, 255, 255});
        else if ( i==4 )
            buttons[4].textSurface = TTF_RenderText_Solid(font, "Level two", {255, 255, 255});
        else if ( i==5 )
            buttons[5].textSurface = TTF_RenderText_Solid(font, "Level three", {255, 255, 255});

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
    pauseMenuButton.rect = {SCREEN_WIDTH - buttonWidth - 50, 25, buttonWidth, buttonHeight};
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



//lvl_buttons'
/*
    Button lvlOne;
    lvlOne.rect = {(screenWidth - buttonWidth) / 2, 300, buttonWidth, buttonHeight};
    lvlOne.textSurface = TTF_RenderText_Solid(font, "Level one.", {255, 255, 255});
    lvlOne.textTexture = SDL_CreateTextureFromSurface(renderer, lvlOne.textSurface);
    lvlOne.isHovered = false;

    Button lvlTwo;
    lvlTwo.rect = {(screenWidth - buttonWidth) / 2, 450, buttonWidth, buttonHeight};
    lvlTwo.textSurface = TTF_RenderText_Solid(font, "Level two.", {255, 255, 255});
    lvlTwo.textTexture = SDL_CreateTextureFromSurface(renderer, lvlTwo.textSurface);
    lvlTwo.isHovered = false;

    Button lvlThree;
    lvlThree.rect = {(screenWidth - buttonWidth) / 2, 600, buttonWidth, buttonHeight};
    lvlThree.textSurface = TTF_RenderText_Solid(font, "Level three.", {255, 255, 255});
    lvlThree.textTexture = SDL_CreateTextureFromSurface(renderer, lvlThree.textSurface);
    lvlThree.isHovered = false;

*/
    bool quit = false;



    Wall wall1(0,700,SCREEN_WIDTH/2,50);

    Wall wall2(300,350,SCREEN_WIDTH/2,50);
    
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                SDL_Point point = {x, y};
                for(int i = 0; i < 5; i++) {
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
                    //isDragging = true;
                    int x, y;
                    prevMouseX = e.motion.x;
                    prevMouseY = e.motion.y;

                    
                    
                    SDL_Point point = {x, y};
                    
                    SDL_GetMouseState(&x, &y);
                    //std::cout << "Mouse clicked at (" << x << ", " << y << ")" << std::endl;

                    prevMouseX = e.motion.x;
                    prevMouseY = e.motion.y;

                            
             
                    for(int i = 0; i < 5; i++) {
                        if (buttons[i].isHovered && gameState == MENU) {
                            // Handle button click
                            if (i == 0) {
                                // Transition to "1st level"
                                //gameState = CHOOSE_LVL;
                                //usleep(1);
                                // Initialize the game state for level 1 here
                            }
                            else if (i == 2) {

                                quit = true;
                            }
                            else if (i == 3)
                            {
                                gameState = LEVEL_1;
                            }
                            else if (i == 4)
                            {
                                gameState = LEVEL_2;
                            }
                            else if (i == 5)
                            {
                                gameState = LEVEL_3;
                            }                         
                        }
                    }
                    //for(int i = 0; i < 5; i++) {
                    //    if (SDL_PointInRect(&point, &buttons[i].rect)) {
                    //        buttons[i].isHovered = true;
                    //    } else {
                    //        buttons[i].isHovered = false;
                    //    }
    
                        //std::cout << "Checking button " << i << ": " << (buttons[i].isHovered ? "Hovered!" : "Not hovered.") << std::endl;
                    //}
                        
                       // lvlOne.isHovered = SDL_PointInRect(&point, &lvlOne.rect);
                       // lvlTwo.isHovered = SDL_PointInRect(&point, &lvlTwo.rect);
                       // lvlThree.isHovered = SDL_PointInRect(&point, &lvlThree.rect);     
                        /*for(int i = 0; i < 5; i++) {
                        std::cout << "Checking button " << i << ": ";
                            if (buttons[i].isHovered) {  
                            std::cout << "Hovered! ";
                            if (i == 0) {
                                std::cout << "Level 1 selected." << std::endl;
                                // Transition to "LEVEL_1"
                                gameState = LEVEL_1;
                                // Initialize the game state for level 1 here
                            } else if (i == 2) {
                                std::cout << "Menu selected." << std::endl;
                                gameState = MENU;
                            } else {
                                // Add similar conditions for other level buttons if needed
                                std::cout << "Unknown button clicked." << std::endl;
                            }
                        } else {
                            std::cout << "Not hovered." << std::endl;
                        }
                    }*/

                    
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
                        ball.setAcceleration(0, 0, 0.0);
                        ball.setVelocity(0, 0, 0.0);
                        ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);
                        ball.resetHitCount();
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }

                     /* lvlOne.isHovered = SDL_PointInRect(&point, &lvlOne.rect);
                        lvlTwo.isHovered = SDL_PointInRect(&point, &lvlTwo.rect);
                        lvlThree.isHovered = SDL_PointInRect(&point, &lvlThree.rect);
                    */
                              
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
            for(int i = 0; i < 5; i++) {
                SDL_SetRenderDrawColor(renderer, buttons[i].isHovered ? 255 : 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &buttons[i].rect);
                SDL_RenderCopy(renderer, buttons[i].textTexture, NULL, &buttons[i].rect);
            }
        } 
        else if (gameState == LEVEL_1) {
            
            //gameState = LEVEL_1;
            //ball.resetHitCount();
            //ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);

            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall1);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall2);
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN;
            }
            ball.move();

            // Draw the hole
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);
            wall1.draw(renderer);
            wall2.draw(renderer);
            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            

            SDL_SetRenderDrawColor(renderer, pauseMenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pauseMenuButton.rect);
            SDL_RenderCopy(renderer, pauseMenuButton.textTexture, NULL, &pauseMenuButton.rect);

        }
        else if (gameState == LEVEL_2) {
            // Draw the walls


            // Add more walls as needed
            // wall3.draw(renderer);
            // ...

            // Handle ball-wall collisions
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall1);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall2);
            // Handle collisions with other walls
            // ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall3);
            // ...
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

        else if (gameState == CHOOSE_LVL) {
            // buttons 
            for(int i = 0; i < 5; i++) {
                //name the buttons
                if (i == 0)
                    buttons[0].textSurface = TTF_RenderText_Solid(font, "LEVEL ONE", {255, 255, 255});
                else if (i == 1)
                    buttons[1].textSurface = TTF_RenderText_Solid(font, "LEVEL TWO", {255, 255, 255});
                else if (i == 2)
                    buttons[2].textSurface = TTF_RenderText_Solid(font, "LEVEL THREE", {255, 255, 255});

                buttons[i].textTexture = SDL_CreateTextureFromSurface(renderer, buttons[i].textSurface);
                buttons[i].isHovered = false;
            }
            // Display the pause screen and buttons
            /*
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, lvlOne.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlOne.rect);
            SDL_RenderCopy(renderer, lvlOne.textTexture, NULL, &lvlOne.rect);


            SDL_SetRenderDrawColor(renderer, lvlTwo.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlTwo.rect);
            SDL_RenderCopy(renderer, lvlTwo.textTexture, NULL, &lvlTwo.rect);

            SDL_SetRenderDrawColor(renderer, lvlThree.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlThree.rect);
            SDL_RenderCopy(renderer, lvlThree.textTexture, NULL, &lvlThree.rect);
        */}
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    for(int i = 0; i < 5; i++) {
        SDL_FreeSurface(buttons[i].textSurface);
        SDL_DestroyTexture(buttons[i].textTexture);
        //SDL_FreeSurface(buttons[i].textSurface);
        //SDL_DestroyTexture(buttons[i].textTexture);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
