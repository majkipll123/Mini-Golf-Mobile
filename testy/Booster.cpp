#include "Booster.h"

Booster::Booster(int radius, int startX, int startY) : x(startX), y(startY), radius(radius), collected(false) {
    // Add any additional initialization for boosters
}

void Booster::draw(SDL_Renderer* renderer) {

    if(!collected)
    {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Green color for boosters
    SDL_Rect rect = {x - radius, y - radius, 2 * radius, 2 * radius};
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }

}
