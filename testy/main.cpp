#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <iostream>
#include <unistd.h>
#include <sqlite3.h>
#include "Ball.h"
#include "Wall.h"
#include "Booster.h"
const float FRICTION = 0.95;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 900

enum GameState {
    MENU,
    CHOOSE_LVL,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    PAUSE_1,
    PAUSE_2,
    PAUSE_3, 
    FINAL_SCREEN,
    FINAL_SCREEN2,
    FINAL_SCREEN3// New game state for the pause screen
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

void initializeDatabase(sqlite3* db, int level) {
    // Create a table if it doesn't exist for the specified level
    std::string createTableSQL = "CREATE TABLE IF NOT EXISTS lvl" + std::to_string(level) + "record (shots INT);";

    char* errorMessage;
    int rc = sqlite3_exec(db, createTableSQL.c_str(), 0, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating table: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }
}


void insertBestShots(sqlite3* db, int level, int bestShots) {
    // Insert the best shots for the specified level
    std::string insertSQL = "INSERT INTO lvl" + std::to_string(level) + "record (shots) VALUES (?);";

    sqlite3_stmt* statement;
    int rc = sqlite3_prepare_v2(db, insertSQL.c_str(), -1, &statement, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_int(statement, 1, bestShots);

    rc = sqlite3_step(statement);
    if (rc != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(statement);
}


int getBestShots(sqlite3* db, int level) {
    // Retrieve the best shots for the specified level
    std::string selectSQL = "SELECT MIN(shots) FROM lvl" + std::to_string(level) + "record;";

    sqlite3_stmt* statement;
    int rc = sqlite3_prepare_v2(db, selectSQL.c_str(), -1, &statement, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    rc = sqlite3_step(statement);
    if (rc == SQLITE_ROW) {
        int retrievedBestShots = sqlite3_column_int(statement, 0);
        sqlite3_finalize(statement);
        return retrievedBestShots;
    } else {
        std::cerr << "Error retrieving data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(statement);
        return 0;
    }
}

void resetRecords(sqlite3* db, int level) {
    // Delete all records for the specified level
    std::string resetSQL = "DELETE FROM lvl" + std::to_string(level) + "record;";
    
    char* errorMessage;
    int rc = sqlite3_exec(db, resetSQL.c_str(), 0, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error resetting records: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "All records for level " << level << " have been deleted." << std::endl;
    }
}




int main(int argc, char* args[]) {

    sqlite3* db;

    int rc = sqlite3_open("your_database_file.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }

    initializeDatabase(db,1);
    initializeDatabase(db,2);
    initializeDatabase(db,3);
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
    Button buttons[8];
    //Button buttons[3];
    //newbutton
    for(int i = 0; i <= 7; i++) {
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
        else if ( i==6 )
            buttons[6].textSurface = TTF_RenderText_Solid(font, "Delete all records", {255, 255, 255});
        else if ( i==7 )
            buttons[7].textSurface = TTF_RenderText_Solid(font, "Level bacd", {255, 255, 255});

        buttons[i].textTexture = SDL_CreateTextureFromSurface(renderer, buttons[i].textSurface);
        buttons[i].isHovered = false;
    }
    Hole hole;

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
    Button pause1MenuButton;
    pause1MenuButton.rect = {SCREEN_WIDTH - buttonWidth - 50, 25, buttonWidth, buttonHeight};
    pause1MenuButton.textSurface = TTF_RenderText_Solid(font, "Pause", {255, 255, 255});
    pause1MenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, pause1MenuButton.textSurface);
    pause1MenuButton.isHovered = false;
    
    Button pause2MenuButton;
    pause2MenuButton.rect = {SCREEN_WIDTH - buttonWidth - 50, 25, buttonWidth, buttonHeight};
    pause2MenuButton.textSurface = TTF_RenderText_Solid(font, "Pause", {255, 255, 255});
    pause2MenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, pause2MenuButton.textSurface);
    pause2MenuButton.isHovered = false;

        Button pause3MenuButton;
    pause3MenuButton.rect = {SCREEN_WIDTH - buttonWidth - 50, 25, buttonWidth, buttonHeight};
    pause3MenuButton.textSurface = TTF_RenderText_Solid(font, "Pause", {255, 255, 255});
    pause3MenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, pause3MenuButton.textSurface);
    pause3MenuButton.isHovered = false;

    // Create the "Resume" and "Back to Menu" buttons for the pause screen
    Button resume1Button;
    resume1Button.rect = {(screenWidth - buttonWidth)/2, 550, buttonWidth, buttonHeight};
    resume1Button.textSurface = TTF_RenderText_Solid(font, "Resume", {255, 255, 255});
    resume1Button.textTexture = SDL_CreateTextureFromSurface(renderer, resume1Button.textSurface);
    resume1Button.isHovered = false;

    Button resume2Button;
    resume2Button.rect = {(screenWidth - buttonWidth)/2, 550, buttonWidth, buttonHeight};
    resume2Button.textSurface = TTF_RenderText_Solid(font, "Resume", {255, 255, 255});
    resume2Button.textTexture = SDL_CreateTextureFromSurface(renderer, resume2Button.textSurface);
    resume2Button.isHovered = false;

    Button resume3Button;
    resume3Button.rect = {(screenWidth - buttonWidth)/2, 550, buttonWidth, buttonHeight};
    resume3Button.textSurface = TTF_RenderText_Solid(font, "Resume", {255, 255, 255});
    resume3Button.textTexture = SDL_CreateTextureFromSurface(renderer, resume3Button.textSurface);
    resume3Button.isHovered = false;

    Button mainMenuButton;
    mainMenuButton.rect = {(screenWidth - buttonWidth)/2,  750, buttonWidth, buttonHeight};
    mainMenuButton.textSurface = TTF_RenderText_Solid(font, "Back to Menu", {255, 255, 255});
    mainMenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, mainMenuButton.textSurface);
    mainMenuButton.isHovered = false;

    Button resetButton;
    resetButton.rect = {(screenWidth - buttonWidth)/2, 700, buttonWidth, buttonHeight};
    resetButton.textSurface = TTF_RenderText_Solid(font, "Delete all records.", {255, 255, 255});
    resetButton.textTexture = SDL_CreateTextureFromSurface(renderer, resetButton.textSurface);
    resetButton.isHovered = false;


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

    // lvl1
    //Booster booster1(10,100,100);

    // lvl2
    Wall wall1(0,700,SCREEN_WIDTH/2,50);
    Wall wall2(300,350,SCREEN_WIDTH/2,50);
    // lvl3
    Wall wall3(SCREEN_WIDTH/3,150,50,SCREEN_HEIGHT);
    Wall wall4(300,550,SCREEN_WIDTH/2,50);
    Wall wall5(SCREEN_WIDTH/3,250,SCREEN_WIDTH/3+50,50);



    Wall empty(0,0,0,0);
    
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                SDL_Point point = {x, y};
                //newbutton
                for(int i = 0; i < 7; i++) {
                    if (SDL_PointInRect(&point, &buttons[i].rect)) {
                        buttons[i].isHovered = true;
                    } else {
                        buttons[i].isHovered = false;
                    }

                }
                pause1MenuButton.isHovered = SDL_PointInRect(&point, &pause1MenuButton.rect);
                resume1Button.isHovered = SDL_PointInRect(&point, &resume1Button.rect);
                pause2MenuButton.isHovered = SDL_PointInRect(&point, &pause1MenuButton.rect);
                resume2Button.isHovered = SDL_PointInRect(&point, &resume2Button.rect);
                mainMenuButton.isHovered = SDL_PointInRect(&point, &mainMenuButton.rect);
                resume3Button.isHovered = SDL_PointInRect(&point, &resume3Button.rect);
                pause3MenuButton.isHovered = SDL_PointInRect(&point, &pause3MenuButton.rect);
                //resetButton.isHovered = SDL_PointInRect(&point, &resetButton.rect);


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

                            
                        //newbutton
                    for(int i = 0; i < 7; i++) {
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
                                ball.setPosition(SCREEN_WIDTH-60,SCREEN_HEIGHT-55);
                            }   
                            else if (i == 6)    
                            {
                                for(int i=1; i<=3; i++)
                                { 
                                
                                resetRecords(db, i);
                                }
                                
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

                    
                    if (pause1MenuButton.isHovered && gameState == LEVEL_1) {
                        // Transition to the PAUSE state
                        gameState = PAUSE_1;
                    }
                    if (resume1Button.isHovered && gameState == PAUSE_1) {
                        // Resume the game
                        gameState = LEVEL_1;
                    }

                    if (pause2MenuButton.isHovered && gameState == LEVEL_2) {
                        // Transition to the PAUSE state
                        gameState = PAUSE_2;
                    }
                    if (resume2Button.isHovered && gameState == PAUSE_2) {
                        // Resume the game
                        gameState = LEVEL_2;

                    }
                    if (pause3MenuButton.isHovered && gameState == LEVEL_3) {
                        // Transition to the PAUSE state
                        gameState = PAUSE_3;
                    }
                    if (resume3Button.isHovered && gameState == PAUSE_3) {
                        // Resume the game
                        gameState = LEVEL_3;

                    }
                    if (mainMenuButton.isHovered && (gameState == FINAL_SCREEN  ||  gameState == PAUSE_1  )) {
                        // Transition back to MENU
                        gameState = MENU;
                        ball.setAcceleration(0, 0, 0.0);
                        ball.setVelocity(0, 0, 0.0);
                        ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);
                        ball.resetHitCount();
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (mainMenuButton.isHovered &&(gameState == FINAL_SCREEN2 || gameState == PAUSE_2)){
                        // Transition back to MENU
                        gameState = MENU;
                        ball.setAcceleration(0, 0, 0.0);
                        ball.setVelocity(0, 0, 0.0);
                        ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);
                        ball.resetHitCount();
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (mainMenuButton.isHovered &&(gameState == FINAL_SCREEN3 || gameState == PAUSE_3)){
                        // Transition back to MENU
                        gameState = MENU;
                        ball.setAcceleration(0, 0, 0.0);
                        ball.setVelocity(0, 0, 0.0);
                        ball.setPosition(SCREEN_WIDTH-25, SCREEN_HEIGHT - 50);
                        ball.resetHitCount();
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                     /* lvlOne.isHovered = SDL_PointInRect(&point, &lvlOne.rect);
                        lvlTwo.isHovered = SDL_PointInRect(&point, &lvlTwo.rect);
                        lvlThree.isHovered = SDL_PointInRect(&point, &lvlThree.rect);
                    */
                              
                }
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT && ball.isready() && gameState == LEVEL_1) {
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
                    if (xVelocity != 0 && yVelocity != 0)
                        ball.increaseHitCount();
                    std::cout << ball.getHitCount() << "\n";
                }
                else if (e.button.button == SDL_BUTTON_LEFT && ball.isready() && gameState == LEVEL_2) {
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
                    if (xVelocity != 0 && yVelocity != 0)
                        ball.increaseHitCount();
                    std::cout << ball.getHitCount() << "\n";
                }
                else if (e.button.button == SDL_BUTTON_LEFT && ball.isready() && gameState == LEVEL_3) {
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
                    if (xVelocity != 0 && yVelocity != 0)
                        ball.increaseHitCount();
                    std::cout << ball.getHitCount() << "\n";
                }
              
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
        SDL_RenderClear(renderer);

        if (gameState == MENU) {
            //newbutton
            for(int i = 0; i < 7; i++) {
                SDL_SetRenderDrawColor(renderer, buttons[i].isHovered ? 255 : 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &buttons[i].rect);
                SDL_RenderCopy(renderer, buttons[i].textTexture, NULL, &buttons[i].rect);
            }
           
        } 
        else if (gameState == LEVEL_1) {
                hole.x = SCREEN_WIDTH/2 ;/* X-coordinate of the hole */;
                hole.y = SCREEN_WIDTH/5+75 ;/* Y-coordinate of the hole */;
                hole.radius = 15 ;
            

            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, empty);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, empty);
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN;

                insertBestShots(db, 1, ball.getHitCount());

                int bestShots = getBestShots(db, 1);
            //    std::cout << "Best shots for Level 1: " <<bestShots<<std::endl;
            }
            ball.move();

            // Draw the hole
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);
            //wall1.draw(renderer);
            //wall2.draw(renderer);
            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            

            SDL_SetRenderDrawColor(renderer, pause1MenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pause1MenuButton.rect);
            SDL_RenderCopy(renderer, pause1MenuButton.textTexture, NULL, &pause1MenuButton.rect);

        }
        else if (gameState == LEVEL_2) {
                hole.x = SCREEN_WIDTH/2 ;/* X-coordinate of the hole */;
                hole.y = SCREEN_WIDTH/5+75 ;/* Y-coordinate of the hole */;
                hole.radius = 15 ;
            //gameState = LEVEL_1;
            //ball.resetHitCount();
            //ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);

            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall1);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall2);
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN2;

                insertBestShots(db, 2, ball.getHitCount());

                int bestShots = getBestShots(db, 2);
               // std::cout << "Best shots for Level 2: " <<bestShots<<std::endl;
            }
            ball.move();

            // Draw the hole
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);
            wall1.draw(renderer);
            wall2.draw(renderer);
            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            

            SDL_SetRenderDrawColor(renderer, pause2MenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pause2MenuButton.rect);
            SDL_RenderCopy(renderer, pause2MenuButton.textTexture, NULL, &pause2MenuButton.rect);
        }
        else if (gameState == LEVEL_3) {
                hole.x = SCREEN_WIDTH/5 ;/* X-coordinate of the hole */;
                hole.y = SCREEN_HEIGHT-50 ;/* Y-coordinate of the hole */;
                hole.radius = 15 ;
            //gameState = LEVEL_1;
            //ball.resetHitCount();
            //ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);

            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall3);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall4);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall5);
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN3;

                insertBestShots(db, 3, ball.getHitCount());

                int bestShots = getBestShots(db, 3);
               // std::cout << "Best shots for Level 2: " <<bestShots<<std::endl;
            }
            ball.move();

            // Draw the hole
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);
            wall3.draw(renderer);
            wall4.draw(renderer);
            wall5.draw(renderer);
            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            

            SDL_SetRenderDrawColor(renderer, pause3MenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pause3MenuButton.rect);
            SDL_RenderCopy(renderer, pause3MenuButton.textTexture, NULL, &pause3MenuButton.rect);
        }

        else if (gameState == PAUSE_1) {
            // Display the pause screen and buttons
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, resume1Button.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &resume1Button.rect);
            SDL_RenderCopy(renderer, resume1Button.textTexture, NULL, &resume1Button.rect);

            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);
        }
        else if (gameState == PAUSE_2) {
            // Display the pause screen and buttons
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, resume2Button.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &resume2Button.rect);
            SDL_RenderCopy(renderer, resume2Button.textTexture, NULL, &resume2Button.rect);

            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);
        }
        else if (gameState == PAUSE_3) {
            // Display the pause screen and buttons
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, resume3Button.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &resume3Button.rect);
            SDL_RenderCopy(renderer, resume3Button.textTexture, NULL, &resume3Button.rect);

            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);
        }
        else if (gameState == FINAL_SCREEN) {
            // Render the final screen
            std::string endText = "Shots: " + std::to_string(ball.getHitCount())+" Your best result: " + std::to_string(getBestShots(db,1));
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
        }
        else if (gameState == FINAL_SCREEN2) {
            // Render the final screen
            std::string endText = "Shots: " + std::to_string(ball.getHitCount())+" Your best result: " + std::to_string(getBestShots(db,2));
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
        }
        else if (gameState == FINAL_SCREEN3) {
            // Render the final screen
            std::string endText = "Shots: " + std::to_string(ball.getHitCount())+" Your best result: " + std::to_string(getBestShots(db,3));
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
        }
            // Handle button click for "Back to Menu"
          
        /*
        if (gameState == LEVEL_1) {
            // Add the "Back to Menu" button in the top-right part of the screen
            SDL_SetRenderDrawColor(renderer, pause1MenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &pause1MenuButton.rect);
            SDL_RenderCopy(renderer, pause1MenuButton.textTexture, NULL, &pause1MenuButton.rect);
        }*/
        

        
        

        else if (gameState == CHOOSE_LVL) {
            // newbutton
            for(int i = 0; i < 7; i++) {
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
    //newbutton
    for(int i = 0; i < 7; i++) {
        SDL_FreeSurface(buttons[i].textSurface);
        SDL_DestroyTexture(buttons[i].textTexture);
        //SDL_FreeSurface(buttons[i].textSurface);
        //SDL_DestroyTexture(buttons[i].textTexture);
    }
    sqlite3_close(db);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}