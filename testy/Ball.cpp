#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cmath>
#include <iostream>
#include "Ball.h"
#include "Wall.h"
#include "Booster.h"
#include "Sand.h"

const float FRICTION = 0.9997;
const float SAND_FRICTION = 0.9994; // 0.9989  albo 0.9997 optymalna wartosc tarcia
const float MIN_VELOCITY = 0.001; //  optymalna wartosc 0.001 minimalna wartosc przed zatrzymaniem

Ball::Ball(int radius, int startX, int startY) : radius(radius), x(startX), y(startY), xVelocity(0), yVelocity(0), zVelocity(0), xAcceleration(0), yAcceleration(0), zAcceleration(0), hitCount(0) {
}

void Ball::move() {

    //xVelocity -= xAcceleration;
    //yVelocity -= yAcceleration;

    if (std::abs(xVelocity) > MIN_VELOCITY || std::abs(yVelocity) > MIN_VELOCITY) {
        xVelocity = xVelocity * FRICTION;
        yVelocity = yVelocity * FRICTION;
        setready(false);
        //std::cout<<"go \n";
    } else {
        stop();
        //std::cout<<"stop \n";
        setready(true);
    };

    x -= xVelocity;
    y -= yVelocity;
}

void Ball::handleCollision(int screenWidth, int screenHeight, const Wall& wall) {
    if (x - radius < 0) {
        x = radius;
        xVelocity = -xVelocity * 0.75;
    } else if (x + radius > screenWidth) {
        x = screenWidth - radius;
        xVelocity = -xVelocity * 0.75;
    }
    if (y - radius < 0) {
        y = radius;
        yVelocity = -yVelocity * 0.75;
    } else if (y + radius > screenHeight) {
        y = screenHeight - radius;
        yVelocity = -yVelocity * 0.75;
    }
    if (x - radius < wall.getX() + wall.getWidth() + 2 &&
        x + radius > wall.getX() &&
        y - radius < wall.getY() + wall.getHeight() + 2 &&
        y + radius > wall.getY()) {

        bool fromLeft = x < wall.getX();
        bool fromRight = x > wall.getX() + wall.getWidth();
        bool fromTop = y < wall.getY();
        bool fromBottom = y > wall.getY() + wall.getHeight();
        
        // Handle collision based on your requirements
        // Example: Reflect the ball's velocity
        if (fromLeft || fromRight) {
            xVelocity = -xVelocity;
        }
        if (fromTop || fromBottom) {
            yVelocity = -yVelocity;
        }
    }
}

void Ball::handleBoosterCollision(Booster& booster) {
    // Check if the ball collides with the booster
    float distance = sqrt(pow(x - booster.getX(), 2) + pow(y - booster.getY(), 2));

    if (distance < radius + booster.getRadius() && !booster.isCollected()) {
        // Handle the collision based on your requirements
        // For example, you can collect the booster and update the ball's properties
        booster.setCollected(true);

        // Calculate the angle between the ball and the booster
        float angle = atan2(booster.getY() - y, booster.getX() - x);

        // Calculate the new velocity components based on the ball's current velocity
        float newVelocityX = xVelocity - cos(angle);
        float newVelocityY = yVelocity - sin(angle);

        // Update the ball's velocity
        setVelocity(newVelocityX, newVelocityY, 0);

        // Additional actions, e.g., increase score, etc.
    }
}

void Ball::handleSlopeCollision(const Sand& sand) {
    // Check if the ball collides with the booster
    float distance = sqrt(pow(x - sand.getX(), 2) + pow(y - sand.getY(), 2));
    //std::cout << "Distance: " << distance << ", Radius Sum: " << (radius + sand.getRadius()) << std::endl;

    if (distance < radius + sand.getRadius()) {
        // Handle the collision based on your requirements
        // For example, you can collect the sand and update the ball's properties
        //sand.setCollected(true);

        // Calculate the angle between the ball and the sand
        float angle = atan2(sand.getY() - y, sand.getX() - x);
        std::cout<<"hit sand\n";

        
        // Calculate the new velocity components based on the ball's current velocity

        // Calculate the new velocity components based on the ball's current velocity
        float newVelocityX = xVelocity *SAND_FRICTION;
        float newVelocityY = yVelocity *SAND_FRICTION;

        // Update the ball's velocity
        setVelocity(newVelocityX, newVelocityY, 0);

        // Additional actions, e.g., increase score, etc.
        //if ((xVelocity <= MIN_VELOCITY) && (yVelocity <= MIN_VELOCITY))
        //{stop();}
    }
}

void Ball::stop() {
    xVelocity = 0;
    yVelocity = 0;
    xAcceleration = 0;
    yAcceleration = 0;
}

void Ball::draw(SDL_Renderer* renderer) {
    filledCircleRGBA(renderer, x, y, radius, 134, 134, 134, 198);
}

void Ball::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void Ball::setAcceleration(float newXAcceleration, float newYAcceleration, float newZAcceleration) {
    xAcceleration = newXAcceleration;
    yAcceleration = newYAcceleration;
}

void Ball::setVelocity(float newXVelocity, float newYVelocity, float newZVelocity) {
    xVelocity = newXVelocity;
    yVelocity = newYVelocity;
}

void Ball::resetHitCount() {
    hitCount = 0;
}

void Ball::applyFriction() {
    // Move friction application to the beginning
    xVelocity *= FRICTION;
    yVelocity *= FRICTION;
}

int Ball::getHitCount() const {
    return hitCount;
}

void Ball::increaseHitCount() {
    hitCount++;
}