
#include "Agent.h"
#include "math.h"
#include "stdio.h"
using namespace std;
Agent::Agent(){}
Agent::Agent(Agent& agent){
    //printf("Constructor called\n");
    direction = agent.direction;
    positionX = agent.positionX;
    positionY = agent.positionY;
    height = agent.height;
    velocity = agent.velocity;
    time = agent.time;
    Id = agent.Id;
    parentId = agent.parentId;
    pruned = agent.pruned;
}
Agent::Agent(float direction, float positionX, float positionY, float height, float velocity, float time, int Id, int parentId,bool pruned = false):direction(direction),positionX(positionX),positionY(positionY),
height(height),velocity(velocity),time(time),Id(Id),parentId(parentId),pruned(pruned){
}


float Agent::DistanceFrom(float x, float y){
    return sqrt((x-positionX)*(x-positionX) + (y-positionY)*(y-positionY));
}
        
float Agent::Energy(float gravity, float friction){
    return ((velocity)*(velocity)/2 + (height)*(gravity))*friction;
}    