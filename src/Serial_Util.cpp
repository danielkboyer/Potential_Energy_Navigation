#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "Util.h"
#include "Serial_Util.h"
#include "random"
#include "math.h"
int intRandSerial(const int & min, const int & max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}
void SwapValueSerial(Agent &a, Agent &b) {
   Agent t = a;
   a = b;
   b = t;
}

void ShuffleSerial(Agent* agents, int count){

    for(int x = 0;x<count;x++){
        int index1 = intRandSerial(0,count-1);
        int index2 = intRandSerial(0,count-1);
        SwapValueSerial(agents[index1],agents[index2]);

    }

}

void Serial_Util::StepAll(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map){
    for(int x = 0;x<inCount;x++){
		int aIndex = x*properties.numberOfDirectionSpawn;
		for(int y = 0;y<properties.numberOfDirectionSpawn;y++){
			float newDirection = in[x].direction - properties.directionSpawnRadius/2 + properties.directionSpawnRadius/(properties.numberOfDirectionSpawn-1) * y;
			//a[aIndex+y] = Agent();
			out[aIndex+y] = AgentStep(in[x],newDirection,properties,map);
            
			//printf("Agent position %f,%f, v=%f\n",out[aIndex+y].positionX,out[aIndex+y].positionY,out[aIndex+y].velocity);
		}
	}
}
// void Serial_Util::RandPrune(Agent* agents, long numberAgents, long agentsToPrune){
//     // shuffle the array to get the points


//     // combine the array 
    
    
//     srand(time(NULL));
//     const int N = numberAgents;
//     long numbers [N];
//     for (int x=0; x!=agentsToPrune;x++){
//         long tmp = (rand()%numberAgents);
//         bool isNotAdded = true;
//         int i=0;
//         while(i<x && isNotAdded) {
//             if(numbers[i] == tmp) isNotAdded = false;
//             i++;
//         }
//         while(isNotAdded)
//            tmp = (rand() % numberAgents);
//         numbers[x] = tmp;
//     }
//     for(int i=0;i<agentsToPrune;i++){
//         agents[numbers[i]].pruned = true;
//         //printf("ID pruned: %ld\n", x);
//     }

//     /// origional random 
//     // srand (100);
//     // long x = 0;
//     // for(int i=0;i<agentsToPrune;i++){
//     //     x = rand()%numberAgents;
//     //     agents[x].pruned = true;
//     //     //printf("ID pruned: %ld\n", x);
//     // }
// }
void Serial_Util::Prune(Agent* agents,Agent* out,long count, long amountToPrune){
    long keepAmount = count - amountToPrune;

    ShuffleSerial(agents,count);

    vector<int> good;
    vector<int> bad;
    int currentIndex = 0;
    
    for(int x = 0;x<count;x++){
        if(isnan(agents[x].velocity) || agents[x].velocity <= 0 || agents[x].pruned == true){
            agents[x].pruned = true;
            bad.push_back(x);
        }
        else{
            good.push_back(x);
        }
    }

    
    for(int x =0 ;x<keepAmount;x++){
        if(x >= good.size()){
            out[x] = agents[bad[x-good.size()]];
            continue;
        }
        out[x] = agents[good[x]];
    }

    out[0].percentage = ((float)(bad.size()))/(float)count;
}



//Must have a non null out agent
Agent Serial_Util::AgentStep(Agent in, float newDirection, Properties properties, Map map){
    //printf("Before\n");
    Agent out;
    out.pruned = false;
    //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
    if(in.pruned == true){
        out.pruned = true;
        return out;
    }
    out = AgentTravel(in,out,newDirection,properties,map);
    //printf("After %f, %f, %f, %f\n",out.positionX,out.positionY,out.time,out.direction);
    out = AgentHeight(in,out,newDirection,properties,map);
    //printf("After Again\n");
    return out;
}

Agent Serial_Util::AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map){  
    out.positionX = in.positionX + cos(newDirection) * properties.travelDistance;
    out.positionY = in.positionY + sin(newDirection) * properties.travelDistance;
    //printf("Agent position %f,%f\n",out->positionX,out->positionY);
    out.direction = newDirection;
    return out;
    
}
//must have out positionX and positionY populated
Agent Serial_Util::AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map){
    out.height = map.GetHeight(out.positionX,out.positionY);
    if(isnan(out.height) || (2*properties.gravity*(in.height - out.height) + in.velocity*in.velocity) < 0){
        //printf("InHeight %f, InVelocity: %f\n",in.height,in.velocity);
        //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
        //printf("Less than zero\n");
        out.pruned = true;
        return out;
    }
    out.velocity = sqrt(2*properties.gravity*(in.height - out.height) + in.velocity*in.velocity) * properties.friction;
    out.time += properties.travelDistance/out.velocity;
    //printf("InHeight %f, InVelocity: %f\n",in.height,in.velocity);
    //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
    return out;
}
