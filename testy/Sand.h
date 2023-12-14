#ifndef SAND_H
#define SAND_H

#include "Wall.h" // Include Wall.h here if needed

class Sand : public Wall {
public:
    Sand(int x, int y, int width, int height); // Constructor for sand slopes
    int getX() const { return x; }
    int getY() const { return y; }
    // You can add any additional member functions or variables specific to Sand

    int getRadius() const { return radius; }
    
private:
    int x;
    int y;
    float radius;
    // You can add any additional member variables specific to Sand
};

#endif
