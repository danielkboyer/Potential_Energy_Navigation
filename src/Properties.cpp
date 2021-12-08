#include "Properties.h"

 
Properties::Properties(int threadCount,float friction, float travelDistance,long maxAgentCount, float agentStartX, float agentStartY,float numberOfDirectionSpawn, float directionSpawnRadius,float gravity):friction(friction),travelDistance(travelDistance),gravity(gravity),
maxAgentCount(maxAgentCount),agentStartX(agentStartX),agentStartY(agentStartY),numberOfDirectionSpawn(numberOfDirectionSpawn),directionSpawnRadius(directionSpawnRadius), threadCount(threadCount){

}

