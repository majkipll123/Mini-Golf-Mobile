#include "Sand.h"

Sand::Sand(int x, int y, int width, int height) : Wall(x, y, width, height) {
    // Additional initialization for Sand if needed
    radius = std::min(width, height) / 2.0f;
}
