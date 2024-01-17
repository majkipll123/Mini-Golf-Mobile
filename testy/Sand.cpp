#include "Sand.h"
#include "iostream"
Sand::Sand(int  x, int  y, int  width, int  height) : Wall(x, y, width, height) {
    // Additional initialization for Sand if needed
    //radius = (width *height)/100000000000; // Radius is half of the minimum dimension
}
void Sand::draw(SDL_Renderer* renderer) {

    SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255); // Green color for boosters
    SDL_Rect rect = {x , y , x, y};
    SDL_RenderFillRect(renderer, &rect);
    std::cout<<x<<"\n"<<y<<"\n"<<radius<<"\n";

}



// Add any additional member functions or variables specific to Sand
