#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "Util.h"
#include "MPI_Util.h"
#if IS_MPI == 1
#include <mpi.h>
#endif
#include <random>
#include <utility>
using namespace std;




int intRandMPI(const int & min, const int & max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}
void SwapValueMPI(Agent &a, Agent &b) {
   Agent t = a;
   a = b;
   b = t;
}
void ShuffleMPI(Agent* agents, int count){
    for(int x = 0;x<count;x++){
        int index1 = intRandMPI(0,count-1);
        int index2 = intRandMPI(0,count-1);
        SwapValueMPI(agents[index1],agents[index2]);
    }

}

void MPI_Util::StepAll(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map){
    // int localInCount = inCount/comm_sz;
    // for(int x = my_rank*localInCount; x < localInCount + my_rank*localInCount; x++){
	// 	int aIndex = x*properties.numberOfDirectionSpawn;
	// 	// printf("aIndex %i, and my rank %i\n",aIndex,my_rank);
    //     for(int y = 0;y<properties.numberOfDirectionSpawn;y++){
	// 		float newDirection = in[x].direction - properties.directionSpawnRadius/2 + properties.directionSpawnRadius/(properties.numberOfDirectionSpawn-1) * y;
	// 		//a[aIndex+y] = Agent();
	// 		out[aIndex+y] = AgentStep(in[x],newDirection,properties,map);
    //     }
    // }
    

    // // for loop 0 to partition
    // int localInCount = inCount/comm_sz;
    // for(int x = 0; x < localInCount; x++){
    //     int aIndex = x*properties.numberOfDirectionSpawn * my_rank;
	// 	printf("aIndex %i, and my rank %i\n",aIndex,my_rank);
    //     for(int y = 0;y<properties.numberOfDirectionSpawn;y++){
	// 		float newDirection = in[x].direction - properties.directionSpawnRadius/2 + properties.directionSpawnRadius/(properties.numberOfDirectionSpawn-1) * y;
    //         out[aIndex+y] = AgentStep(in[x],newDirection,properties,map);
	// 		printf("Agent position %f,%f\n",a[aIndex+y].positionX,a[aIndex+y].positionY);
	// 	}
	// }
}

void MPI_Util::Prune(Agent* agents,Agent* out,long count, long amountToPrune){
    srand (100);
    long x = 0;
    for(int i=0;i<amountToPrune;i++){
        x = rand()%count;
        agents[x].pruned = true;
        //printf("ID pruned: %ld\n", x);
    }
}
//Must have a non null out agent
Agent MPI_Util::AgentStep(Agent in, float newDirection, Properties properties, Map map){
    //printf("Before\n");
    Agent out;
    out.pruned = false;
    //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
    
    out = AgentTravel(in,out,newDirection,properties,map);
    //printf("After %f, %f, %f, %f\n",out.positionX,out.positionY,out.time,out.direction);
    out = AgentHeight(in,out,newDirection,properties,map);
    //printf("After Again\n");
    return out;
}

Agent MPI_Util::AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map){  
    out.positionX = in.positionX + cos(newDirection) * properties.travelDistance;
    out.positionY = in.positionY + sin(newDirection) * properties.travelDistance;
    //printf("Agent position %f,%f\n",out->positionX,out->positionY);
    out.direction = newDirection;
    return out;
    
}

//must have out positionX and positionY populated
Agent MPI_Util::AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map){
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
