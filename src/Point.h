#pragma once
using namespace std;
#include<iostream>
class Point
{
#ifdef __CUDACC__
#define CUDA_CALLABLE_MEMBER __host__ __device__
#else
#define CUDA_CALLABLE_MEMBER
#endif 
    public:
        CUDA_CALLABLE_MEMBER Point(int x, int y, float height);
        CUDA_CALLABLE_MEMBER Point();
        int x;
        int y;
        float height;
        
};