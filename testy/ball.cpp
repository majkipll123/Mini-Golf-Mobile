#include <SDL2/SDL.h>

class Ball {
public:
    Ball(int radius, int startX, int startY) : radius(radius), x(startX), y(startY), xVelocity(5), yVelocity(5) {
    }

    void move() {
        x += xVelocity;
        y += yVelocity;
    }

    void handleCollision(int screenWidth, int screenHeight) {
        if (x - radius < 0 || x + radius > screenWidth) {
            xVelocity = -xVelocity;
        }

        if (y - radius < 0 || y + radius > screenHeight) {
            yVelocity = -yVelocity;
        }
    }

    void draw(SDL_Surface* surface) {
        filledCircleRGBA(surface, x, y, radius, 255, 0, 0, 255); // Rysuj czerwoną piłkę
    }

private:
    int radius;
    int x;
    int y;
    int xVelocity;
    int yVelocity;
};
