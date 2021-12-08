#include "Properties.h"

 Properties::Properties(){};
Properties::Properties(float friction, float travelDistance,long maxAgentCount, float agentStartX, float agentStartY,float numberOfDirectionSpawn, float directionSpawnRadius,float gravity):friction(friction),travelDistance(travelDistance),gravity(gravity),
maxAgentCount(maxAgentCount),agentStartX(agentStartX),agentStartY(agentStartY),numberOfDirectionSpawn(numberOfDirectionSpawn),directionSpawnRadius(directionSpawnRadius){

}

