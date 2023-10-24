#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cmath>

#include "Ball.h"

const float FRICTION = 0.95; // I dont understand why the friction doesnt work lolz
const float MIN_VELOCITY = 0.01; // Minimum velocity before stopping

Ball::Ball(int radius, int startX, int startY) : radius(radius), x(startX), y(startY), xVelocity(0), yVelocity(0), xAcceleration(0), yAcceleration(0) {
}

void Ball::move() {
    // Update velocity
    xVelocity += xAcceleration;
    yVelocity += yAcceleration;

    // Apply friction
    xAcceleration *= FRICTION;
    yAcceleration *= FRICTION;



    // Stop the ball if its velocity is close to zero
    if (std::abs(xVelocity) < MIN_VELOCITY) {
        xVelocity = 0;
    }

    if (std::abs(yVelocity) < MIN_VELOCITY) {
        yVelocity = 0;
    }

    // Update position
    x += xVelocity;
    y += yVelocity;
}

void Ball::handleCollision(float screenWidth, float screenHeight) {
    if (x + radius < 10 || x + radius > screenWidth) {
        xVelocity = -xVelocity;
        xAcceleration = -xAcceleration;
    }

    if (y + radius < 10 || y + radius > screenHeight) {
        yVelocity = -yVelocity;
        yAcceleration = -yAcceleration;
    }
}

void Ball::draw(SDL_Renderer* renderer) {
    filledCircleRGBA(renderer, x, y, radius, 255, 0, 0, 255);
}

void Ball::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void Ball::setAcceleration(float newXAcceleration, float newYAcceleration) {
    xAcceleration = newXAcceleration;
    yAcceleration = newYAcceleration;
}

void Ball::setVelocity(float newXVelocity, float newYVelocity) {
    xVelocity = newXVelocity;
    yVelocity = newYVelocity;
}

void Ball::applyFriction() {
    // Move friction application to the beginning
    xVelocity *= FRICTION;
    yVelocity *= FRICTION;
}
