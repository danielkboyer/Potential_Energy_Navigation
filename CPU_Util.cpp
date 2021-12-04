#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "Util.h"
#include "CPU_Util.h"
class CPU_Util
{

    void CPU_Util::CheckPrune(Agent out, Properties properties){
        out.pruned = true;
    }
    //Must have a non null out agent
    void CPU_Util::AgentStep(Agent in, Agent out, float newDirection, Properties properties, Map map){
        AgentTravel(in,out,newDirection,properties,map);
        AgentHeight(in,out,newDirection,properties,map);
    }

    void CPU_Util::AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map){  
        out.positionX = in.positionX + cos(newDirection) * properties.travelDistance;
        out.positionY = in.positionY + sin(newDirection) * properties.travelDistance;
        out.time += properties.travelDistance/out.velocity;
        out.direction = newDirection;
        
    }
    //must have out positionX and positionY populated
    void CPU_Util::AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map){

        out.height = map.GetHeight(out.positionX,out.positionY);
        
        out.velocity = sqrt(2*properties.gravity*(in.height - out.height) + in.velocity*in.velocity) * properties.friction;
    }
};