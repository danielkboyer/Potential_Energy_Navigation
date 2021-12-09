#pragma once
using namespace std;
#include "Point.h"
class Map
{

    private:
        
        
    public:
        Map();
        
        void ReadFile(string fileName);
        //takes a float x and y and returns the points surrounding that point
        float GetHeight(float x, float y);

        float GetHeight(int x, int y);

        float GetPointDistance();
        float _pointDistance;
        Point *points;
        int _height;
        int _width;
};