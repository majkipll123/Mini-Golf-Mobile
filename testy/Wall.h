#ifndef WALL_H
#define WALL_H

#include <SDL2/SDL.h>

class Wall {
public:
    Wall(int x, int y, int width, int height);
    void draw(SDL_Renderer* renderer);
    
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int x, y, width, height;
};

#endif
