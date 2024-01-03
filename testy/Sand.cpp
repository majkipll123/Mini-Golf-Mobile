#include "Sand.h"

Sand::Sand(int  x, int  y, int  width, int  height) : Wall(x, y, width, height) {
    // Additional initialization for Sand if needed
    radius = std::min(width, height); // Radius is half of the minimum dimension
}
void Sand::draw(SDL_Renderer* renderer) {

    
    
    SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255); // Green color for boosters
    SDL_Rect rect = {x - radius, y - radius, 2 * radius, 2 * radius};
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    

}



// Add any additional member functions or variables specific to Sand
