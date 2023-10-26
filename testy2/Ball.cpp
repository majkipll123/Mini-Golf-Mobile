#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <cmath>
 
#include <iostream>
#include "Ball.h"

const float FRICTION = 0.9989; // I dont understand why the friction doesnt work lolz
const float MIN_VELOCITY = 0.001; // Minimum velocity before stopping

Ball::Ball(int radius, int startX, int startY) : radius(radius), x(startX), y(startY), xVelocity(0), yVelocity(0), zVelocity(0), xAcceleration(0), yAcceleration(0), zAcceleration(0){
}

void Ball::move() {

    //xVelocity -= xAcceleration;
    //yVelocity -= yAcceleration;
    

    if(std::abs(xVelocity)> MIN_VELOCITY || std::abs(yVelocity) > MIN_VELOCITY) {
        xVelocity = xVelocity* FRICTION;
        yVelocity = yVelocity* FRICTION;
        //isready(false);
        //std::cout<<"go \n";
    }
    else {
        stop();
        //std::cout<<"stop \n";
        //tutaj dodac funcje ktora pozwala dodac przyspieszenie 
        //isready(true);
    };
    


    x -= xVelocity;
    y -= yVelocity;
    /*
    std::cout<<x<<"\n";
    std::cout<<y<<"\n";
    std::cout<<xAcceleration<<" xaccel \n";
    std::cout<<yAcceleration<<" yaccel \n";
    std::cout<<xVelocity<<" xvelaccel \n";
    std::cout<<yVelocity<<" yvelaccel \n"; //symulowac dokladne obicia za pomoca przewidywania czasowego git 
    */
}

void Ball::handleCollision(int screenWidth, int screenHeight) {
   if (x - radius < 0) {
        x = radius;
        xVelocity = -xVelocity*0.75;
    }
    else if (x + radius > screenWidth) {
        x = screenWidth - radius;
        xVelocity = -xVelocity*0.75;
    }
    if (y - radius < 0) {
        y = radius;
        yVelocity = -yVelocity*0.75;
    }
    else if (y + radius > screenHeight) {
        y = screenHeight - radius;
        yVelocity = -yVelocity*0.75;
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

void Ball::applyFriction() {
    // Move friction application to the beginning
    xVelocity *= FRICTION;
    yVelocity *= FRICTION;
}
/*
bool Ball::isready(bool isready){
    ready= isready
    
}*/

