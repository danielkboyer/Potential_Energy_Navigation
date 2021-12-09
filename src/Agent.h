#pragma once
using namespace std;
class Agent
{

#ifdef __CUDACC__
#define CUDA_CALLABLE_MEMBER __host__ __device__
#else
#define CUDA_CALLABLE_MEMBER
#endif 
    public:
        CUDA_CALLABLE_MEMBER Agent(float direction, float positionX, float positionY, float height, float velocity, float time, int Id, int parentId,float percentage,bool pruned);

        CUDA_CALLABLE_MEMBER Agent();
        CUDA_CALLABLE_MEMBER Agent(Agent& agent);
        CUDA_CALLABLE_MEMBER void Set(float direction, float positionX, float positionY, float height, float velocity, float time, int Id, int parentId,float percentage,bool pruned = false);
        float direction;
        float positionX;
        float positionY;
        float height;
        float velocity;
        float percentage;
        //in seconds
        float time;
        int Id;
        int parentId;
        bool pruned;

        CUDA_CALLABLE_MEMBER float DistanceFrom(float x, float y);
        CUDA_CALLABLE_MEMBER float Energy(float gravity, float friction);
};