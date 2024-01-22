#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
//#include <SDL_mixer.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <unistd.h>
#include <sqlite3.h>
#include "Ball.h"
#include "Wall.h"
#include "Booster.h"
#include "Sand.h"   
const float FRICTION = 0.95;

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 1000

enum GameState {
    MENU,
    CHOOSE_LVL,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    LEVEL_5,
    LEVEL_6,
    PAUSE_1,
    PAUSE_2,
    PAUSE_3, 
    PAUSE_4,
    PAUSE_5,
    PAUSE_6,
    FINAL_SCREEN,
    FINAL_SCREEN2,
    FINAL_SCREEN3,
    FINAL_SCREEN4,
    FINAL_SCREEN5,
    FINAL_SCREEN6,
    INSTRUCTUINS// New game state for the pause screen
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
/*
void playSound() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    // Load sound from a file (you need to provide a valid path to your sound file)
    Mix_Chunk* sound = Mix_LoadWAV("/home/majkipll123/Documents/Github/Mini-Golf-Mobile/testy/bat hit against wall.wav");
    if (!sound) {
        std::cerr << "Failed to load sound file: " << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        SDL_Quit();
        return;
    }

    // Play the sound
    Mix_PlayChannel(-1, sound, 0);

    // Wait until the sound is finished playing (optional)
    while (Mix_Playing(-1) != 0) {
        // Do nothing, just wait
    }
}
*/
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
void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while (x >= y) {
        SDL_RenderDrawLine(renderer, centerX - x, centerY + y, centerX + x, centerY + y);
        SDL_RenderDrawLine(renderer, centerX - x, centerY - y, centerX + x, centerY - y);

        SDL_RenderDrawLine(renderer, centerX - y, centerY + x, centerX + y, centerY + x);
        SDL_RenderDrawLine(renderer, centerX - y, centerY - x, centerX + y, centerY - x);

        y++;

        if (radiusError < 0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x + 1);
        }
    }
}


SDL_Surface* renderTextLines(TTF_Font* font, const std::vector<std::string>& textLines) {
    int totalHeight = 0;
    for (const std::string& line : textLines) {
        totalHeight += TTF_FontHeight(font);
    }

    SDL_Surface* textSurface = SDL_CreateRGBSurface(0, 600, totalHeight, 32, 0, 0, 0, 0);

    if (!textSurface) {
        std::cerr << "Failed to create text surface: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_Rect textRect = {0, 0, 0, 0};

    for (const std::string& line : textLines) {
        SDL_Surface* lineSurface = TTF_RenderText_Solid(font, line.c_str(), {255, 255, 255});

        if (!lineSurface) {
            std::cerr << "Failed to render text surface: " << TTF_GetError() << std::endl;
            SDL_FreeSurface(textSurface);
            return nullptr;
        }

        SDL_BlitSurface(lineSurface, nullptr, textSurface, &textRect);
        textRect.y += TTF_FontHeight(font);
        SDL_FreeSurface(lineSurface);
    }

    return textSurface;
}
int main(int argc, char* args[]) {

    sqlite3* db;

    int rc = sqlite3_open("your_database_file.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }
    for (int i=0; i<=6; i++)
        initializeDatabase(db,i);
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

    // Create a font your font path
    TTF_Font* font = TTF_OpenFont("/home/majkipll123/Documents/Github/Mini-Golf-Mobile/mini-golf-mobile-game/Beautiful Bride.otf", 54);

    // Create the buttons
    const int buttonWidth = 400;
    const int buttonHeight = 60;
    const int screenWidth = SCREEN_WIDTH;
    Button buttons[5];
    Button levels_buttons[6];
    //newbutton idk why i can make buttons[i] but cant make any more buttons in for bc cant access them later (?)
    for(int i = 0; i <= 3; i++) {
        buttons[i].rect = {(screenWidth - buttonWidth)/2, i*100+100, buttonWidth, buttonHeight};
        //name the buttons
        if ( i==0 )
            buttons[0].textSurface = TTF_RenderText_Solid(font, "Practise offline", {255, 255, 255});
        else if ( i==1 )
            buttons[1].textSurface = TTF_RenderText_Solid(font, "Options", {255, 255, 255});
        else if ( i==2 )
            buttons[2].textSurface = TTF_RenderText_Solid(font, "Quit", {255, 255, 255});
        else if ( i==3 )
            buttons[3].textSurface = TTF_RenderText_Solid(font, "Delete all records", {255, 255, 255});
        else if ( i==4 )
            buttons[4].textSurface = TTF_RenderText_Solid(font, "Level bacd", {255, 255, 255});

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

    Button pause4MenuButton;
    pause4MenuButton.rect = {SCREEN_WIDTH - buttonWidth - 50, 25, buttonWidth, buttonHeight};
    pause4MenuButton.textSurface = TTF_RenderText_Solid(font, "Pause", {255, 255, 255});
    pause4MenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, pause4MenuButton.textSurface);
    pause4MenuButton.isHovered = false;

    Button pause5MenuButton;
    pause5MenuButton.rect = {SCREEN_WIDTH - buttonWidth +50, 25, buttonWidth-30, buttonHeight};
    pause5MenuButton.textSurface = TTF_RenderText_Solid(font, "Pause", {255, 255, 255});
    pause5MenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, pause5MenuButton.textSurface);
    pause5MenuButton.isHovered = false;
    Button pause6MenuButton;
    pause6MenuButton.rect = {SCREEN_WIDTH - buttonWidth - 50, 25, buttonWidth, buttonHeight};
    pause6MenuButton.textSurface = TTF_RenderText_Solid(font, "Pause", {255, 255, 255});
    pause6MenuButton.textTexture = SDL_CreateTextureFromSurface(renderer, pause6MenuButton.textSurface);
    pause6MenuButton.isHovered = false;
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

    Button resume4Button;
    resume4Button.rect = {(screenWidth - buttonWidth)/2, 550, buttonWidth, buttonHeight};
    resume4Button.textSurface = TTF_RenderText_Solid(font, "Resume", {255, 255, 255});
    resume4Button.textTexture = SDL_CreateTextureFromSurface(renderer, resume4Button.textSurface);
    resume4Button.isHovered = false;

    Button resume5Button;
    resume5Button.rect = {(screenWidth - buttonWidth)/2, 550, buttonWidth, buttonHeight};
    resume5Button.textSurface = TTF_RenderText_Solid(font, "Resume", {255, 255, 255});
    resume5Button.textTexture = SDL_CreateTextureFromSurface(renderer, resume5Button.textSurface);
    resume5Button.isHovered = false;

    Button resume6Button;
    resume6Button.rect = {(screenWidth - buttonWidth)/2, 550, buttonWidth, buttonHeight};
    resume6Button.textSurface = TTF_RenderText_Solid(font, "Resume", {255, 255, 255});
    resume6Button.textTexture = SDL_CreateTextureFromSurface(renderer, resume6Button.textSurface);
    resume6Button.isHovered = false;

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

    Button lvlOne;
    lvlOne.rect = {(screenWidth - buttonWidth) / 2, 250, buttonWidth, buttonHeight};
    lvlOne.textSurface = TTF_RenderText_Solid(font, "Level one.", {255, 255, 255});
    lvlOne.textTexture = SDL_CreateTextureFromSurface(renderer, lvlOne.textSurface);
    lvlOne.isHovered = false;

    Button lvlTwo;
    lvlTwo.rect = {(screenWidth - buttonWidth) / 2, 310, buttonWidth, buttonHeight};
    lvlTwo.textSurface = TTF_RenderText_Solid(font, "Level two.", {255, 255, 255});
    lvlTwo.textTexture = SDL_CreateTextureFromSurface(renderer, lvlTwo.textSurface);
    lvlTwo.isHovered = false;

    Button lvlThree;
    lvlThree.rect = {(screenWidth - buttonWidth) / 2, 370, buttonWidth, buttonHeight};
    lvlThree.textSurface = TTF_RenderText_Solid(font, "Level three.", {255, 255, 255});
    lvlThree.textTexture = SDL_CreateTextureFromSurface(renderer, lvlThree.textSurface);
    lvlThree.isHovered = false;

    Button lvlFour;
    lvlFour.rect = {(screenWidth - buttonWidth) / 2, 430, buttonWidth, buttonHeight};
    lvlFour.textSurface = TTF_RenderText_Solid(font, "Level Four.", {255, 255, 255});
    lvlFour.textTexture = SDL_CreateTextureFromSurface(renderer, lvlFour.textSurface);
    lvlFour.isHovered = false;
        
    Button lvlFive;
    lvlFive.rect = {(screenWidth - buttonWidth) / 2, 490, buttonWidth, buttonHeight};
    lvlFive.textSurface = TTF_RenderText_Solid(font, "Level Five.", {255, 255, 255});
    lvlFive.textTexture = SDL_CreateTextureFromSurface(renderer, lvlFive.textSurface);
    lvlFive.isHovered = false;

    Button lvlSix;
    lvlSix.rect = {(screenWidth - buttonWidth) / 2, 550, buttonWidth, buttonHeight};
    lvlSix.textSurface = TTF_RenderText_Solid(font, "Level Six.", {255, 255, 255});
    lvlSix.textTexture = SDL_CreateTextureFromSurface(renderer, lvlSix.textSurface);
    lvlSix.isHovered = false;

    bool quit = false;


    //Sand sandSlope(5, 5, SCREEN_WIDTH/2, 300);
    // lvl1
    Booster booster1(10,SCREEN_WIDTH/4,SCREEN_HEIGHT/2);
    Booster booster2(10,(SCREEN_WIDTH*3)/4,SCREEN_HEIGHT/2);  
    Booster booster3(10,(SCREEN_WIDTH-50),SCREEN_HEIGHT/2);  
    Booster booster4(10,(SCREEN_WIDTH+50)/2,SCREEN_HEIGHT/2);  
    //Booster booster2(10,(SCREEN_WIDTH*3)/4,SCREEN_HEIGHT/2);  
    // lvl2// wall is posx posy widthx widthy
    Wall wall1(0,350,(SCREEN_WIDTH*3)/4,50);
    Wall wall2(SCREEN_WIDTH/3,650,SCREEN_WIDTH,50);

    
    //lvl4
    Wall wall6(100,0,50,SCREEN_HEIGHT-150);
    Wall wall7(150,SCREEN_HEIGHT-200,SCREEN_WIDTH/4,50);
    Wall wall8(250+(SCREEN_WIDTH/4),100,50,SCREEN_HEIGHT);
    Wall wall9(100+(SCREEN_WIDTH/4),0,50,SCREEN_HEIGHT-150);
    Wall wall10(450,100,SCREEN_WIDTH/4,50);
    Wall wall11(400+(SCREEN_WIDTH/4),100,50,SCREEN_HEIGHT-200);
    //Wall wall11(150+(SCREEN_WIDTH/2),SCREEN_HEIGHT-200,-50,-SCREEN_HEIGHT);
    
    // lvl
    Wall wall3(SCREEN_WIDTH/4,150,50,SCREEN_HEIGHT);
    Wall wall4(400,540,SCREEN_WIDTH,50);
    Wall wall5((SCREEN_WIDTH/3)-20,220,SCREEN_WIDTH/2,50);

    //lvl5
    Wall wall12((SCREEN_WIDTH-25)/2,SCREEN_HEIGHT/5,50,SCREEN_HEIGHT/2);

    //lvl6
    

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
                for(int i = 0; i <=2; i++) {
                    if (SDL_PointInRect(&point, &levels_buttons[i].rect)) {
                        levels_buttons[i].isHovered = true;
                    } else {
                        levels_buttons[i].isHovered = false;
                    }

                }
                pause1MenuButton.isHovered = SDL_PointInRect(&point, &pause1MenuButton.rect);
                resume1Button.isHovered = SDL_PointInRect(&point, &resume1Button.rect);
                pause2MenuButton.isHovered = SDL_PointInRect(&point, &pause1MenuButton.rect);
                resume2Button.isHovered = SDL_PointInRect(&point, &resume2Button.rect);
                mainMenuButton.isHovered = SDL_PointInRect(&point, &mainMenuButton.rect);
                resume3Button.isHovered = SDL_PointInRect(&point, &resume3Button.rect);
                pause3MenuButton.isHovered = SDL_PointInRect(&point, &pause3MenuButton.rect);
                resume6Button.isHovered = SDL_PointInRect(&point, &resume6Button.rect);
                pause4MenuButton.isHovered = SDL_PointInRect(&point, &pause4MenuButton.rect);
                resume4Button.isHovered = SDL_PointInRect(&point, &resume4Button.rect);
                pause5MenuButton.isHovered = SDL_PointInRect(&point, &pause5MenuButton.rect);
                resume5Button.isHovered = SDL_PointInRect(&point, &resume5Button.rect);
                pause6MenuButton.isHovered = SDL_PointInRect(&point, &pause6MenuButton.rect);
                lvlOne.isHovered = SDL_PointInRect(&point, &lvlOne.rect);
                lvlThree.isHovered = SDL_PointInRect(&point, &lvlThree.rect);
                lvlTwo.isHovered = SDL_PointInRect(&point, &lvlTwo.rect);
                lvlFour.isHovered = SDL_PointInRect(&point, &lvlFour.rect);
                lvlFive.isHovered = SDL_PointInRect(&point, &lvlFive.rect);
                lvlSix.isHovered = SDL_PointInRect(&point, &lvlSix.rect);
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
                    for(int i = 0; i < 5; i++) {
                        if (buttons[i].isHovered && gameState == MENU) {
                            // Handle button click
                            if (i == 0) {
                                // Transition to "1st level"
                                gameState = CHOOSE_LVL;
                                //usleep(1);
                                // Initialize the game state for level 1 here
                            }
                            else if (i == 1)
                            {
                                gameState = INSTRUCTUINS;
                                
                            }
                            else if (i == 2) {
                                
                                quit = true;
                            }
                            else if (i == 3)
                            {
                                for(int i=1; i<=6; i++)
                                { 
                                resetRecords(db, i);
                                }

                            }
                            else if (i == 4)
                            {
                                gameState = LEVEL_2;
                                ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);
                            }
                            else if (i == 5)
                            {
                                gameState = LEVEL_3;
                                ball.setPosition(SCREEN_WIDTH-60,SCREEN_HEIGHT-55);
                            }   
                            else if (i == 6)    
                            {

                                
                            }                  
                        }
                    }            
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
                    if (resume4Button.isHovered && gameState == PAUSE_4) {
                        // Resume the game
                        gameState = LEVEL_4;
                    }
                    if (pause4MenuButton.isHovered && gameState == LEVEL_4) {
                        // Transition to the PAUSE state
                        gameState = PAUSE_4;
                    }
                    if (resume5Button.isHovered && gameState == PAUSE_5) {
                        // Resume the game
                        gameState = LEVEL_5;
                    }
                    if (pause5MenuButton.isHovered && gameState == LEVEL_5) {
                        // Transition to the PAUSE state
                        gameState = PAUSE_5;
                    }
                    if (resume6Button.isHovered && gameState == PAUSE_6) {
                        // Resume the game
                        gameState = LEVEL_6;
                    }
                    if (pause6MenuButton.isHovered && gameState == LEVEL_6) {
                        // Transition to the PAUSE state
                        gameState = PAUSE_6;
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
                    else if (mainMenuButton.isHovered &&(gameState == FINAL_SCREEN4 || gameState == PAUSE_4)){
                        // Transition back to MENU
                        gameState = MENU;
                        ball.setAcceleration(0, 0, 0.0);
                        ball.setVelocity(0, 0, 0.0);
                        ball.setPosition(SCREEN_WIDTH-25, SCREEN_HEIGHT - 50);
                        ball.resetHitCount();
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (mainMenuButton.isHovered &&(gameState == INSTRUCTUINS)){
                        // Transition back to MENU
                        gameState = MENU;
                        ball.setAcceleration(0, 0, 0.0);
                        ball.setVelocity(0, 0, 0.0);
                        ball.setPosition(SCREEN_WIDTH-25, SCREEN_HEIGHT - 50);
                        ball.resetHitCount();
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (mainMenuButton.isHovered &&(gameState == FINAL_SCREEN5 || gameState == PAUSE_5)){
                        // Transition back to MENU
                        gameState = MENU;
                        ball.setAcceleration(0, 0, 0.0);
                        ball.setVelocity(0, 0, 0.0);
                        ball.setPosition(SCREEN_WIDTH-25, SCREEN_HEIGHT - 50);
                        ball.resetHitCount();
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (mainMenuButton.isHovered &&(gameState == FINAL_SCREEN6 || gameState == PAUSE_6)){
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
                    else if (lvlOne.isHovered && gameState == CHOOSE_LVL){
                        // Transition back to MENU
                        gameState = LEVEL_1;
                        booster1.setCollected(false);
                        ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (lvlTwo.isHovered && gameState == CHOOSE_LVL){
                        // Transition back to MENU
                        gameState = LEVEL_2;
                        booster1.setCollected(false);
                        ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (lvlThree.isHovered && gameState == CHOOSE_LVL){
                        // Transition back to MENU
                        gameState = LEVEL_3;
                        booster1.setCollected(false);
                        ball.setPosition(SCREEN_WIDTH-60,SCREEN_HEIGHT-55);
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (lvlFour.isHovered && gameState == CHOOSE_LVL){
                        // Transition back to MENU
                        gameState = LEVEL_4;
                        booster1.setCollected(false);
                        ball.setPosition((SCREEN_WIDTH+25)/2,SCREEN_HEIGHT-55);
                        
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (lvlFive.isHovered && gameState == CHOOSE_LVL){
                        // Transition back to MENU
                        gameState = LEVEL_5;
                        booster1.setCollected(false);
                        ball.setPosition(50,SCREEN_HEIGHT/7);
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (lvlSix.isHovered && gameState == CHOOSE_LVL){
                        // Transition back to MENU
                        gameState = LEVEL_6;
                        booster1.setCollected(false);
                        ball.setPosition(SCREEN_WIDTH-60,SCREEN_HEIGHT-55);
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
                    else if (mainMenuButton.isHovered && gameState == CHOOSE_LVL){
                        // Transition back to MENU
                        gameState = MENU;
                        booster1.setCollected(false);
                        ball.setPosition(SCREEN_WIDTH-60,SCREEN_HEIGHT-55);
                       // ball.resetHitCount();

                        // Add any additional logic for transitioning back to MENU here
                    }
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
                else if (e.button.button == SDL_BUTTON_LEFT && ball.isready() && gameState == LEVEL_4) {
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
                else if (e.button.button == SDL_BUTTON_LEFT && ball.isready() && gameState == LEVEL_5) {
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
                else if (e.button.button == SDL_BUTTON_LEFT && ball.isready() && gameState == LEVEL_6) {
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
            for(int i = 0; i < 5; i++) {
                SDL_SetRenderDrawColor(renderer, buttons[i].isHovered ? 255 : 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &buttons[i].rect);
                SDL_RenderCopy(renderer, buttons[i].textTexture, NULL, &buttons[i].rect);
            }
        
        } 
        else if (gameState == CHOOSE_LVL) {
            // Display buttons
            
            /*for(int i = 0; i <=2 ; i++) {
                std::cout<<"tak";

                SDL_SetRenderDrawColor(renderer, levels_buttons[i].isHovered ? 255 : 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &levels_buttons[i].rect);
                SDL_RenderCopy(renderer, levels_buttons[i].textTexture, NULL, &levels_buttons[i].rect);
            }*/
            SDL_SetRenderDrawColor(renderer, lvlOne.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlOne.rect);
            SDL_RenderCopy(renderer, lvlOne.textTexture, NULL, &lvlOne.rect);

            SDL_SetRenderDrawColor(renderer, lvlTwo.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlTwo.rect);
            SDL_RenderCopy(renderer, lvlTwo.textTexture, NULL, &lvlTwo.rect);

            SDL_SetRenderDrawColor(renderer, lvlThree.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlThree.rect);
            SDL_RenderCopy(renderer, lvlThree.textTexture, NULL, &lvlThree.rect);

            SDL_SetRenderDrawColor(renderer, lvlFour.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlFour.rect);
            SDL_RenderCopy(renderer, lvlFour.textTexture, NULL, &lvlFour.rect);

            SDL_SetRenderDrawColor(renderer, lvlFive.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlFive.rect);
            SDL_RenderCopy(renderer, lvlFive.textTexture, NULL, &lvlFive.rect);

            SDL_SetRenderDrawColor(renderer, lvlSix.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &lvlSix.rect);
            SDL_RenderCopy(renderer, lvlSix.textTexture, NULL, &lvlSix.rect);

            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);
            
        }
        else if (gameState == LEVEL_1) {
                hole.x = SCREEN_WIDTH/2 ;/* X-coordinate of the hole */;
                hole.y = SCREEN_WIDTH/5+75 ;/* Y-coordinate of the hole */;
                hole.radius = 15 ;

            ball.handleBoosterCollision(booster1);
            //ball.handleCollision(SCREEN_WIDTH,SCREEN_HEIGHT, sandSlope);
            //ball.handleSlopeCollision(sandSlope);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, empty);
            SDL_SetRenderDrawColor(renderer, 255, 204, 102, 255); // Sand color (adjust as needed)
            
            
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN;

                insertBestShots(db, 1, ball.getHitCount());

                int bestShots = getBestShots(db, 1);
            //    std::cout << "Best shots for Level 1: " <<bestShots<<std::endl;
            }
            ball.move();

            // Draw the hole
            
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);
            
            //filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);

            //wall1.draw(renderer);
            //wall2.draw(renderer);
            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            //sandSlope.draw(renderer);

           
            SDL_SetRenderDrawColor(renderer, pause1MenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pause1MenuButton.rect);
            SDL_RenderCopy(renderer, pause1MenuButton.textTexture, NULL, &pause1MenuButton.rect);
            
            SDL_RenderPresent(renderer);
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
                hole.x = SCREEN_WIDTH/6 ;/* X-coordinate of the hole */;
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
        else if (gameState == LEVEL_5) {
                hole.x = SCREEN_WIDTH*3/4 ;/* X-coordinate of the hole */;
                hole.y = (SCREEN_HEIGHT+20)/6 ;/* Y-coordinate of the hole */;
                hole.radius = 15 ;
            //gameState = LEVEL_1;
            //ball.resetHitCount();
            //ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall6);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall7);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall8);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall9);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall10);
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall11);
            
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN4;

                insertBestShots(db, 4, ball.getHitCount());

                int bestShots = getBestShots(db, 4);
               // std::cout << "Best shots for Level 2: " <<bestShots<<std::endl;
            }
            ball.move();
            if (!booster4.isCollected())
                booster4.draw(renderer);   
            if (!booster3.isCollected())
                booster3.draw(renderer); 
            ball.handleBoosterCollision(booster4);
            ball.handleBoosterCollision(booster3);
            // Draw the hole
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);
            wall6.draw(renderer);
            wall7.draw(renderer);
            wall8.draw(renderer);
            wall9.draw(renderer);
            wall10.draw(renderer);
            wall11.draw(renderer);
            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            

            SDL_SetRenderDrawColor(renderer, pause5MenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pause5MenuButton.rect);
            SDL_RenderCopy(renderer, pause5MenuButton.textTexture, NULL, &pause5MenuButton.rect);
        }
        else if (gameState == LEVEL_4) {
                hole.x = (SCREEN_WIDTH+25)/2 ;/* X-coordinate of the hole */;
                hole.y = 100 ;/* Y-coordinate of the hole */;
                hole.radius = 15 ;
            //gameState = LEVEL_1;
            //ball.resetHitCount();
            //ball.setPosition(SCREEN_WIDTH/2, SCREEN_HEIGHT - 50);

           
            ball.handleCollision(SCREEN_WIDTH, SCREEN_HEIGHT, wall12);
            if (isCollision(ball.getX(), ball.getY(), ball.getRadius(), hole)) {
                gameState = FINAL_SCREEN5;

                insertBestShots(db, 5, ball.getHitCount());

                int bestShots = getBestShots(db, 5);
               // std::cout << "Best shots for Level 2: " <<bestShots<<std::endl;
            }
            ball.move();
            ball.handleBoosterCollision(booster1);
            ball.handleBoosterCollision(booster2);
            // Draw the hole
            filledCircleColor(renderer, hole.x, hole.y, hole.radius, 0xFF0000FF);
            if (!booster1.isCollected())
                booster1.draw(renderer);   
            if (!booster2.isCollected())
                booster2.draw(renderer);   
            wall12.draw(renderer);
            // Draw the ball after the hole to ensure it's not covered
            ball.draw(renderer);
            

            SDL_SetRenderDrawColor(renderer, pause4MenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pause4MenuButton.rect);
            SDL_RenderCopy(renderer, pause4MenuButton.textTexture, NULL, &pause4MenuButton.rect);
        }
        else if (gameState == LEVEL_6) {
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
                gameState = FINAL_SCREEN6;

                insertBestShots(db, 6, ball.getHitCount());

                int bestShots = getBestShots(db, 6);
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
            

            SDL_SetRenderDrawColor(renderer, pause6MenuButton.isHovered ? 0 : 0, 0, 255, 125);
            SDL_RenderFillRect(renderer, &pause6MenuButton.rect);
            SDL_RenderCopy(renderer, pause6MenuButton.textTexture, NULL, &pause6MenuButton.rect);
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
        else if (gameState == PAUSE_4) {
            // Display the pause screen and buttons
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, resume4Button.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &resume4Button.rect);
            SDL_RenderCopy(renderer, resume4Button.textTexture, NULL, &resume4Button.rect);

            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);
        }
        else if (gameState == PAUSE_5) {
            // Display the pause screen and buttons
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, resume5Button.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &resume5Button.rect);
            SDL_RenderCopy(renderer, resume5Button.textTexture, NULL, &resume5Button.rect);

            SDL_SetRenderDrawColor(renderer, mainMenuButton.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &mainMenuButton.rect);
            SDL_RenderCopy(renderer, mainMenuButton.textTexture, NULL, &mainMenuButton.rect);
        }
        else if (gameState == PAUSE_6) {
            // Display the pause screen and buttons
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // Semi-transparent black background
            SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &pauseRect);

            SDL_SetRenderDrawColor(renderer, resume6Button.isHovered ? 255 : 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &resume6Button.rect);
            SDL_RenderCopy(renderer, resume6Button.textTexture, NULL, &resume6Button.rect);

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
        else if (gameState == FINAL_SCREEN4) {
            // Render the final screen
            std::string endText = "Shots: " + std::to_string(ball.getHitCount())+" Your best result: " + std::to_string(getBestShots(db,4));
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
        else if (gameState == FINAL_SCREEN5) {
            // Render the final screen
            std::string endText = "Shots: " + std::to_string(ball.getHitCount())+" Your best result: " + std::to_string(getBestShots(db,5));
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
        else if (gameState == FINAL_SCREEN6) {
            // Render the final screen
            std::string endText = "Shots: " + std::to_string(ball.getHitCount())+" Your best result: " + std::to_string(getBestShots(db,6));
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
        else if (gameState == INSTRUCTUINS) {
            
            // Render the final screen  to \n \n  \n so try to collect them as they will help you to move around the map";
            std::vector<std::string> textLines = {
                "Welcome to the mini golf mobile!",
                "This is my first project game in C++",
                "For now, there are 6 levels", 
                "in the practice offline tab",
                "The movement is simple, just drag and shoot!",
                "There are yellow boosters on the map",
                "So try to collect them! Have Fun!"
                "also there is a reset all records button",
                "watch out with that!"
            };

            // Render text lines onto a surface
            SDL_Surface* endSurface = renderTextLines(font, textLines);

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
 
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    //newbutton
    for(int i = 0; i < 5; i++) {
        SDL_FreeSurface(buttons[i].textSurface);
        SDL_DestroyTexture(buttons[i].textTexture);
        //SDL_FreeSurface(levels_buttons[i].textSurface);
        //SDL_DestroyTexture(levels_buttons[i].textTexture);
    }
    //Mix_FreeChunk(sound);
    sqlite3_close(db);
    TTF_CloseFont(font);
    //Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}