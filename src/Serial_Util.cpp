#include <omp.h>

#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "Util.h"
#include "Serial_Util.h"
#include "random"
#include "math.h"
#include "string.h"
#include "timer.h"
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
#if OMP == false
void ShuffleSerial(Agent* agents, int count,int threadCount){
      for(int x = 0;x<count;x++){
        int index1 = intRandSerial(0,count-1);
        int index2 = intRandSerial(0,count-1);
        SwapValueSerial(agents[index1],agents[index2]);
    }
}
#else
void ShuffleSerial(Agent* agents, int count,int threadCount){
    int x = 0;
    int perThread = floor(count/threadCount);
#pragma omp parallel num_threads(threadCount) default(none) private(x) shared(perThread,count,agents,threadCount)
{
    int myRank = omp_get_thread_num();
    int min = myRank*perThread;
    int max = min+perThread-1;
    if(myRank == threadCount-1){
        max = threadCount-1;
    }

    for(x = min;x<max;x++){
        int index1 = intRandSerial(min,max);
        int index2 = intRandSerial(min,max);
        SwapValueSerial(agents[index1],agents[index2]);
    }


}
}
#endif
void Serial_Util::StepAll(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map){
    int x, y, aIndex;
    float newDirection;
    
    // make this for loop parallel, this is the outer loop of the sorting function
#   pragma omp parallel for num_threads(properties.threadCount) default(none) private(x,y,aIndex,newDirection) shared(out,in,properties,map,inCount)
    for(x = 0;x<inCount;x++){
        //printf("loop 1 parrallel or not %d \n", omp_get_num_threads());
        aIndex = x*properties.numberOfDirectionSpawn;
		for(y = 0;y<properties.numberOfDirectionSpawn;y++){
			newDirection = in[x].direction - properties.directionSpawnRadius/2 + properties.directionSpawnRadius/(properties.numberOfDirectionSpawn-1) * y;
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

// Origional
// void Serial_Util::Prune(Agent* agents,Agent* out,long count, long amountToPrune, Properties properties){
//     long keepAmount = count - amountToPrune;
//     ShuffleSerial(agents,count);
//     vector<int> good;
//     vector<int> bad;
//     for(int x = 0;x<count;x++){
//         if(isnan(agents[x].velocity) || agents[x].velocity <= 0 || agents[x].pruned == true){
//             agents[x].pruned = true;
//             bad.push_back(x);
//         }
//         else{
//             good.push_back(x);
//         }
//     }
//     for(int x =0 ;x<keepAmount;x++){
//         if(x >= good.size()){
//             out[x] = agents[bad[x-good.size()]];
//             continue;
//         }
//         out[x] = agents[good[x]];
//     }
//     out[0].percentage = ((float)(bad.size()))/(float)count;
// }


// created from GPU
void Serial_Util::Prune(Agent* agents, Agent* out, long count, long amountToPrune,Properties properties){

    if(amountToPrune == 0){
        memcpy(out,agents,sizeof(Agent) * count);
        return;
    }

    long keepAmount = count - amountToPrune;

    ShuffleSerial(agents,count,properties.threadCount);
    // for(int x = 0;x<count;x++){
    //     printf("check agent: %f,%f\n",agents[x].positionX,agents[x].positionY);
    // }
    int* good = new int[count];
    int* bad = new int[count];
    int x;
    int badCount = 0;
double first_prune=0;
double second_prune = 0;
double first_end = 0;
double second_end = 0;
GET_TIME(first_prune);
#pragma omp parallel num_threads(properties.threadCount) default(none) private(x) shared(out,amountToPrune,keepAmount,count,agents,good,bad) reduction(+:badCount)
{
#   pragma omp for 
    for(x = 0;x<count;x++){
        if(isnan(agents[x].velocity) || agents[x].velocity <= 0 || agents[x].pruned == true){
            agents[x].pruned = true;
            bad[x] = x;
            good[x] = -1;
            badCount+=1;
        }
        else{
            bad[x] = -1;
            good[x] = x;
        }
    }
}
GET_TIME(first_end);
GET_TIME(second_prune)

int index = 0;
for(int x = 0;x<keepAmount;x++){
    if(good[x] != -1){
        out[index++] = agents[good[x]];
    }
}

for(int x = 0;index<keepAmount;x++){
    if(bad[x] != -1)
        out[index++] = agents[bad[x]];
}

// for(int x = 0;x<keepAmount;x++){
//         printf("New check agent: %f,%f\n",out[x].positionX,out[x].positionY);
//     }
GET_TIME(second_end);
// #   pragma omp for
//     for(int x = 0;x<count;x++){
//         if(bad[x] != -1)
//             badCount += 1;
//     }



//printf("Prune time 1 %f\n",first_end-first_prune);
//printf("Prune time 2 %f\n",second_end-second_prune);
//printf("badCount %i\n",badCount);

    out[0].percentage = ((float)(badCount))/(float)count;
    //printf("good count %i, bad count %i, amount to keep %i, and ammount to prune %i\n", goodCount, badCount,keepAmount,amountToPrune);
    delete[] good;
    delete[] bad;
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