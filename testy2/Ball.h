#ifndef BALL_H
#define BALL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

class Ball {
public:
    Ball(int radius, int startX, int startY);
    void move();
    void handleCollision(int screenWidth, int screenHeight);
    void draw(SDL_Renderer* renderer);
    void setPosition(int newX, int newY);
    void setAcceleration(int newXAcceleration, int newYAcceleration);
    void applyFriction();

private:
    int radius;
    int x;
    int y;
    float xVelocity;
    float yVelocity;
    float xAcceleration;
    float yAcceleration;
};

#endif
