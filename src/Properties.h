#pragma once
using namespace std;
class Properties
{
#ifdef __CUDACC__
#define CUDA_CALLABLE_MEMBER __host__ __device__
#else
#define CUDA_CALLABLE_MEMBER
#endif 
    public:
        CUDA_CALLABLE_MEMBER Properties();
        CUDA_CALLABLE_MEMBER Properties(int threadCount,float friction, float travelDistance,long maxAgentCount, float agentStartX, float agentStartY,float numberOfDirectionSpawn, float directionSpawnRadius,float gravity = 9.81);
        float numberOfDirectionSpawn;
        float directionSpawnRadius;
        float friction;
        float travelDistance;
        float gravity;
        long maxAgentCount;
        float agentStartX;
        float agentStartY;
        int threadCount;
};
