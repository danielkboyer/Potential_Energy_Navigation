
#include "Agent.h"


    
Agent::Agent(float direction, float positionX, float positionY, float height, float velocity, Agent *parent){
    _direction = direction;
    _positionX = positionX;
    _positionY = positionY;
    _height = height;
    _velocity = velocity;
    _parent = parent;
}
        
