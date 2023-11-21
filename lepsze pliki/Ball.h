#ifndef BALL2_H
#define BALL2_H

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

class Ball {
public:
    Ball(int radius, int startX, int startY);
    void move();
    void stop();
    void handleCollision(int screenWidth, int screenHeight);
    void draw(SDL_Renderer* renderer);
    void setPosition(float newX, float newY);
    void setAcceleration(float newXAcceleration, float newYAcceleration, float newZAcceleration);
    void setVelocity(float newXVelocity, float newYVelocity, float);
    void applyFriction();
    
    bool isready() const {
        return ready;
    }

    void setready(bool value) {
        ready = value;
    }
    int getHitCount() const;

    void increaseHitCount();
    
    int getX() const { return x; }
    int getY() const { return y; }
    int getRadius() const { return radius; }

private:
    int radius;
    int hitCount;
    bool ready;
    float x;
    float y;
    float xVelocity;
    float yVelocity;
    float zVelocity;
    float xAcceleration;
    float yAcceleration;
    float zAcceleration;
};

#endif
