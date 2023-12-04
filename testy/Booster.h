#ifndef BOOSTER_H
#define BOOSTER_H

#include <SDL2/SDL.h>

class Booster {
public:
    Booster(int radius, int startX, int startY);
    void draw(SDL_Renderer* renderer);
    
    int getX() const { return x; }
    int getY() const { return y; }

    // Add any additional functionality specific to boosters
    bool isCollected() const { return collected; }
    void setCollected(bool value) { collected = value; }

private:
    int x, y;
    int radius;
    bool collected;
};

#endif
