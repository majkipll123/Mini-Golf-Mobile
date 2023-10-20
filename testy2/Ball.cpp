#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cmath>

#include "Ball.h"

const float FRICTION = 0.95;



Ball::Ball(int radius, int startX, int startY) : radius(radius), x(startX), y(startY), xVelocity(0), yVelocity(0), xAcceleration(0), yAcceleration(0) {
}

void Ball::move() {

    xVelocity += xAcceleration; // Dodaj przyspieszenie do prędkości
    yVelocity += yAcceleration;
    x -= xVelocity;
    y -= yVelocity;
}

void Ball::handleCollision(int screenWidth, int screenHeight) {
    if (x - radius < 0 || x + radius > screenWidth) {
        xVelocity = -xVelocity;
    }

    if (y - radius < 0 || y + radius > screenHeight) {
        yVelocity = -yVelocity;
    }
}

void Ball::draw(SDL_Renderer* renderer) {
    filledCircleRGBA(renderer, x, y, radius, 255, 0, 0, 255);
}

void Ball::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

void Ball::setAcceleration(int newXAcceleration, int newYAcceleration) {
    xAcceleration = newXAcceleration;
    yAcceleration = newYAcceleration;
}

void Ball::applyFriction() {
    xVelocity *= FRICTION;
    yVelocity *= FRICTION;
}
