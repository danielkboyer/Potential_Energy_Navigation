#pragma once
using namespace std;
class Properties
{

    public:
        Properties();
        Properties(float friction, float travelDistance,long maxAgentCount, float agentStartX, float agentStartY,float numberOfDirectionSpawn, float directionSpawnRadius,float gravity = 9.81);
        float numberOfDirectionSpawn;
        float directionSpawnRadius;
        float friction;
        float travelDistance;
        float gravity;
        long maxAgentCount;
        float agentStartX;
        float agentStartY;
};
