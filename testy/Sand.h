#ifndef SAND_H
#define SAND_H

#include "Wall.h" // Include Wall.h here if needed

class Sand : public Wall {
public:
    Sand(int  x, int  y, int  width, int  height); // Constructor for sand slopes
    int  getX() const { return x; }
    int  getY() const { return y; }
    int  getRadius() const { return radius; }
    void draw(SDL_Renderer* renderer);

private:
    int  x;
    int  y;
    int  radius;
};

#endif
