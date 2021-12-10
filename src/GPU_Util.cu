#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "GPU_Util.h"
#include "random"
#include <fstream>
#include <curand.h>
#include <curand_kernel.h> 
#include "Point.h"
#include "Agent.h"
#include "math.h"
#include "stdio.h"
#include "thrust/device_vector.h"

using namespace std;



__host__ __device__ Agent::Agent(){}
__host__ __device__ Agent::Agent(Agent& agent){
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
    percentage = agent.percentage;
}
__host__ __device__ Agent::Agent(float direction, float positionX, float positionY, float height, float velocity, float time, int Id, int parentId,float percentage,bool pruned = false):direction(direction),positionX(positionX),positionY(positionY),
height(height),velocity(velocity),time(time),Id(Id),parentId(parentId),pruned(pruned),percentage(percentage){
}


__host__ __device__ float Agent::DistanceFrom(float x, float y){
    return sqrt((x-positionX)*(x-positionX) + (y-positionY)*(y-positionY));
}
        
__host__ __device__ float Agent::Energy(float gravity, float friction){
    return ((velocity)*(velocity)/2 + (height)*(gravity))*friction;
}    

__global__
void GPU_Step(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map);

__device__
int intRandGPU(int id,const int & min, const int & max) {
    curandState state;
    curand_init((unsigned long long)clock() + id, 0, 0, &state);
    double rand1 = (curand_uniform_double(&state)) *(max-min+0.999999);
    return (int)truncf(rand1);
}
__device__
void SwapValueGPU(Agent &a, Agent &b) {
   Agent t = a;
   a = b;
   b = t;
}
__device__
void ShuffleGPU(Agent* agents, int count, int id){

    for(int x = 0;x<count;x++){
        int index1 = intRandGPU(id,0,count-1);
        int index2 = intRandGPU(id,0,count-1);
        SwapValueGPU(agents[index1],agents[index2]);

    }

}

__global__ void GPU_Step(Agent* in, int inCount, Agent* out, int outCount, int apt, Properties* properties, Map* map,Point* points){

    int x = blockDim.x;

    int x_b = blockIdx.x;
    int y_b = blockIdx.y;
   
    
    int x_t = threadIdx.x;

    int mainId = ((x_b * 512)+(y_b*x*512)+x_t)*apt;
    //printf("Main ID: %i\n",mainId);
    int outId =mainId * properties->numberOfDirectionSpawn;
    for(int x = 0;x<apt && (outId+ x) < outCount;x++){
        //printf("Looping, mainid:%i\n",mainId);
		int aIndex = x*properties->numberOfDirectionSpawn;
        //printf("Number of direction spawn %f\n",properties->numberOfDirectionSpawn);
        //printf("Travel Distance %f\n",properties->travelDistance);
		for(int y = 0;y<(int)properties->numberOfDirectionSpawn;y++){
            
            int inIndex = mainId + x;
            int outIndex = outId + aIndex + y;
            
            //printf("Main ID: %i, OutIndex: %i, InIndex: %i\n",mainId,outIndex,inIndex);
            Agent outAgent;
            outAgent.pruned = false;
            Agent inAgent = in[inIndex];
            if(inAgent.pruned == true){
                outAgent.pruned = true;
                out[outIndex] = outAgent;
                continue;
            }
			float newDirection = inAgent.direction - properties->directionSpawnRadius/2 + properties->directionSpawnRadius/(properties->numberOfDirectionSpawn-1) * y;
			
            
            //printf("In Agent = PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",inAgent.positionX,inAgent.positionY,inAgent.velocity,inAgent.height,properties->gravity,properties->friction);
    
            outAgent.positionX = inAgent.positionX + cos(newDirection) * properties->travelDistance;
            outAgent.positionY = inAgent.positionY + sin(newDirection) * properties->travelDistance;
            //printf("Out Agent = PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",outAgent.positionX,outAgent.positionY,outAgent.velocity,outAgent.height,properties->gravity,properties->friction);
    
            outAgent.direction = newDirection;

            //Get the height
            int startX = floor(inAgent.positionX/map->_pointDistance);
            int startY = floor(inAgent.positionY/map->_pointDistance);
            //printf("StartX: %d , StartY: %d\n",startX,startY);

            if(startX < 0 || startX +1 >= map->_width || startY -1 < 0 || startY >= map->_height){
                outAgent.height = NAN;
                outAgent.pruned = true;
                
                out[outIndex] = outAgent;
                continue;
            }
            float xPoint = inAgent.positionX/map->_pointDistance - startX;
            float yPoint = inAgent.positionY/map->_pointDistance - startY;

            outAgent.height = (map->_pointDistance - xPoint)*(map->_pointDistance - yPoint)*(points[startY*map->_width+startX].height) + 
                    (map->_pointDistance)*(map->_pointDistance - yPoint)*(points[startY*map->_width+ startX+1].height) +
                    (map->_pointDistance - xPoint)*(map->_pointDistance)*(points[(startY-1)*map->_width+ startX].height) +
                    (map->_pointDistance)*(map->_pointDistance)*(points[(startY-1)*map->_width + startX+1].height);

            

            //done getting height
            if(isnan(outAgent.height) || (2*properties->gravity*(inAgent.height - outAgent.height) + inAgent.velocity*inAgent.velocity) < 0){
              
                outAgent.pruned = true;
                
                out[outIndex] = outAgent;
                continue;
            }
    

            outAgent.velocity = sqrt(2*properties->gravity*(inAgent.height - outAgent.height) + inAgent.velocity*inAgent.velocity) * properties->friction;
            outAgent.time += properties->travelDistance/outAgent.velocity;
            //printf("Out Agent = PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",outAgent.positionX,outAgent.positionY,outAgent.velocity,outAgent.height,properties->gravity,properties->friction);
    
            out[outIndex] = outAgent;

        }
        
    }

    //printf("x:%i,y:%i,z:%i  x_t:%i,y_t:%i,z_y:%i\n",x,y,z,x_t,y_t,z_t);


}
void GPU_Util::StepAll(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map){

    //10
    //printf("OUT COUNT: %i\n",outCount);
    //10
    //10
    int atp = 9;
    int gridNumber = (int)ceil(((float)inCount/(float)atp)/(float)512);
    //printf("Grid Number: %i\n",gridNumber);
    dim3 DimGrid(gridNumber,gridNumber,1);
    dim3 DimBlock(512,1,1);

    Agent* in_d;
    Agent* out_d;
    Properties* properties_d;
    Map* map_d;
    Point* points_d;


    //printf("PROPERTIES : NUMBER %f\n",properties.numberOfDirectionSpawn);

    ///POINTS MEMORY
    //cudaMalloc((void **)&map_d,sizeof(Map));
    //cudaMemcpy(map_d,&map,sizeof(Map),cudaMemcpyHostToDevice);

    cudaMalloc((void **)&points_d,sizeof(Point)*map._width*map._height);
    cudaMemcpy(points_d,map.points,sizeof(Point)*map._width*map._height,cudaMemcpyHostToDevice);

    cudaMalloc((void **)&map_d, sizeof(Map));
    cudaMemcpy(map_d,&map, sizeof(Map),cudaMemcpyHostToDevice);
    //cudaMemcpy(temp_data,map.points,sizeof(Point)*map._width*map._height,cudaMemcpyHostToDevice);
    

    //End Point Memory
    cudaMalloc((void **)&properties_d,sizeof(Properties));

    cudaMalloc((void **)&out_d,outCount*sizeof(Agent));
    cudaMalloc((void **)&in_d,inCount*sizeof(Agent));

    
    cudaMemcpy(properties_d,&properties,sizeof(Properties),cudaMemcpyHostToDevice);

    cudaMemcpy(in_d,in,inCount*sizeof(Agent),cudaMemcpyHostToDevice);

    GPU_Step<<<DimGrid,DimBlock>>>(in_d,inCount,out_d,outCount,atp,properties_d,map_d,points_d);
    cudaDeviceSynchronize();
    cudaMemcpy(out,out_d,outCount*sizeof(Agent),cudaMemcpyDeviceToHost);
    cudaFree(in_d);
    cudaFree(out_d);
    cudaFree(properties_d);
    cudaFree(map_d);
    cudaFree(points_d);
    // for(int x = 0;x<outCount;x++){
    //     printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out[x].positionX,out[x].positionY,out[x].velocity,out[x].height,properties.gravity,properties.friction);
    // }
}


__global__ void PruneGPU(Agent* agents,Agent* out,long count,long pruneAmountTotal,long apt, long aptP){
    int x = blockDim.x;

    int x_b = blockIdx.x;
    int y_b = blockIdx.y;
   
    
    int x_t = threadIdx.x;
    long keepAmount = apt - aptP;
    int mainId = ((x_b * 512)+(y_b*x*512)+x_t)*apt;
    if(mainId < count){
        return;
    }
    int outId = ((x_b * 512)+(y_b*x*512)+x_t)*keepAmount;
    
    Agent* my_local_agents = new Agent[apt];
    for(int x = 0;x<apt;x++){
        if(mainId+x < count){
        my_local_agents[x] = agents[mainId+x];
        }
        else{
            Agent dummy;
            dummy.pruned = true;
            my_local_agents[x] = dummy;
        }
    }

    ShuffleGPU(my_local_agents,apt,mainId);
    int* good = new int[apt];
    int* bad = new int[apt];
    int goodCount = 0;
    int badCount = 0;
    for(int x = 0;x<apt;x++){
        if(isnan(my_local_agents[x].velocity) || my_local_agents[x].velocity <= 0 || my_local_agents[x].pruned == true){
            my_local_agents[x].pruned = true;
            bad[badCount++] = x;
        }
        else{
            bad[goodCount++] = x;
        }
    }
    
    for(int x =0 ;x<keepAmount;x++){
        if(x >= goodCount){
            if(mainId+x < pruneAmountTotal){
                out[outId+x] = my_local_agents[bad[x-goodCount]];
            }
            continue;
        }
        if(mainId+x < pruneAmountTotal){
            out[outId+x] = my_local_agents[good[x]];
        }
    }
    out[outId].percentage = ((float)(badCount))/(float)apt;
    delete[] my_local_agents;
    delete[] good;
    delete[] bad;
}



void GPU_Util::Prune(Agent* agents,Agent* out,long count, long amountToPrune, Properties properties){

    int apt = 16;
    int amountToAdd = count%512;
    
    int gridNumber = (int)ceil(((float)(count+amountToAdd)/(float)apt)/(float)512);
    int randomPerThread = (count+amountToAdd)/512/pow(gridNumber,2);
    //printf("Grid Number: %i\n",gridNumber);
    dim3 DimGrid(gridNumber,gridNumber,1);
    dim3 DimBlock(512,1,1);

    Agent* in_d;
    Agent* out_d;


    cudaMalloc((void **)&out_d,(count-amountToPrune)*sizeof(Agent));
    cudaMalloc((void **)&in_d,count*sizeof(Agent));

    
    cudaMemcpy(out_d,&out,sizeof(Agent)*(count-amountToPrune),cudaMemcpyHostToDevice);

    cudaMemcpy(in_d,agents,count*sizeof(Agent),cudaMemcpyHostToDevice);

    PruneGPU<<<DimGrid,DimBlock>>>(in_d,out_d,count,(count-amountToPrune),apt,randomPerThread);
    cudaDeviceSynchronize();
    cudaMemcpy(out,out_d,(count-amountToPrune)*sizeof(Agent),cudaMemcpyDeviceToHost);
    cudaFree(in_d);
    cudaFree(out_d);
}




//Must have a non null out agent

Agent GPU_Util::AgentStep(Agent in, float newDirection, Properties properties, Map map){
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

Agent GPU_Util::AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map){  
    out.positionX = in.positionX + cos(newDirection) * properties.travelDistance;
    out.positionY = in.positionY + sin(newDirection) * properties.travelDistance;
    //printf("Agent position %f,%f\n",out->positionX,out->positionY);
    out.direction = newDirection;
    return out;
    
}
//must have out positionX and positionY populated

Agent GPU_Util::AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map){
    
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
