
#include "Agent.h"
#include "math.h"

Agent::Agent(){}
Agent::Agent(float direction, float positionX, float positionY, float height, float velocity, float time, int Id, int parentId,bool pruned = false):direction(direction),positionX(positionX),positionY(positionY),
height(height),velocity(velocity),parentId(parentId),time(time),pruned(pruned),Id(Id){
}

float Agent::DistanceFrom(float x, float y){
    return sqrt((x-positionX)*(x-positionX) + (y-positionY)*(y-positionY));
}
        
