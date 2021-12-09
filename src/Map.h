#pragma once
using namespace std;
#include "Point.h"
class Map
{
#ifdef __CUDACC__
#define CUDA_CALLABLE_MEMBER __host__ __device__
#define CUDA_CALLABLE_MEMBERH __host__
#else
#define CUDA_CALLABLE_MEMBERH
#define CUDA_CALLABLE_MEMBER
#endif 
    private:
        
        
    public:
        CUDA_CALLABLE_MEMBER Map();
        
        CUDA_CALLABLE_MEMBERH void ReadFile(string fileName);
        //takes a float x and y and returns the points surrounding that point
        CUDA_CALLABLE_MEMBER float GetHeight(float x, float y);

        CUDA_CALLABLE_MEMBER float GetHeight(int x, int y);

        CUDA_CALLABLE_MEMBER float GetPointDistance();
        float _pointDistance;
        Point *points;
        int _height;
        int _width;
};