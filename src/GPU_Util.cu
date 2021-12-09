#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "GPU_Util.h"
#include "random"


#include "Agent.h"
#include "math.h"
#include "stdio.h"
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

//__device__
int intRandGPU(const int & min, const int & max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}
//__device__
void SwapValueGPU(Agent &a, Agent &b) {
   Agent t = a;
   a = b;
   b = t;
}
//__device__
void ShuffleGPU(Agent* agents, int count){

    for(int x = 0;x<count;x++){
        int index1 = intRandGPU(0,count-1);
        int index2 = intRandGPU(0,count-1);
        SwapValueGPU(agents[index1],agents[index2]);

    }

}
__constant__ Properties PROPERTIES;
__global__ void GPU_Step(Agent* in, int inCount, Agent* out, int outCount, int apt, Map* map){

    int x = blockDim.x;

    int x_b = blockIdx.x;
    int y_b = blockIdx.y;
   
    
    int x_t = threadIdx.x;

    int mainId = (x_b * 512)+(y_b*x*512)+x_t;
    int outId =mainId * PROPERTIES.numberOfDirectionSpawn;
    for(int x = 0;x<apt && (mainId*apt + x) < inCount;x++){
        printf("Looping, mainid:%i\n",mainId);
		int aIndex = x*PROPERTIES.numberOfDirectionSpawn;
        printf("Number of direction spawn %i\n",PROPERTIES.numberOfDirectionSpawn);
		for(int y = 0;y<PROPERTIES.numberOfDirectionSpawn;y++){
            
			float newDirection = in[x].direction - PROPERTIES.directionSpawnRadius/2 + PROPERTIES.directionSpawnRadius/(PROPERTIES.numberOfDirectionSpawn-1) * y;
			
            int inIndex = mainId + x;
            int outIndex = outId + aIndex + y;
            
            printf("Creating agent. OutIndex: %i\n",outIndex);
            Agent outAgent;
            outAgent.pruned = false;
            Agent inAgent = in[inIndex];
            printf("In Agent = PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",inAgent.positionX,inAgent.positionY,inAgent.velocity,inAgent.height,PROPERTIES.gravity,PROPERTIES.friction);
    
            outAgent.positionX = inAgent.positionX + cos(newDirection) * PROPERTIES.travelDistance;
            outAgent.positionY = inAgent.positionY + sin(newDirection) * PROPERTIES.travelDistance;
        
            outAgent.direction = newDirection;

            //Get the height
            int startX = floor(x/map->_pointDistance);
            int startY = floor(y/map->_pointDistance);
            //printf("StartX: %d , StartY: %d\n",startX,startY);
            if(startX < 0)
                outAgent.height = NAN;
            if(startX +1 >= map->_width)
                outAgent.height = NAN;
            if(startY -1 < 0)
                outAgent.height = NAN;
            if(startY >= map->_height)
                outAgent.height = NAN;
            //printf("x %f, y %f\n",x/_pointDistance,y/_pointDistance);
            float xPoint = x/map->_pointDistance - startX;
            float yPoint = y/map->_pointDistance - startY;
            //printf("yPoint %f, xPoint %f\n",yPoint,xPoint);
            outAgent.height = (map->_pointDistance - xPoint)*(map->_pointDistance - yPoint)*(map->points[startY][startX].height) + 
                    (map->_pointDistance)*(map->_pointDistance - yPoint)*(map->points[startY][startX+1].height) +
                    (map->_pointDistance - xPoint)*(map->_pointDistance)*(map->points[startY-1][startX].height) +
                    (map->_pointDistance)*(map->_pointDistance)*(map->points[startY-1][startX+1].height);


            //done getting height
            if(isnan(outAgent.height) || (2*PROPERTIES.gravity*(inAgent.height - outAgent.height) + inAgent.velocity*inAgent.velocity) < 0){
              
                outAgent.pruned = true;

                out[outIndex] = outAgent;
                continue;
            }
    

            outAgent.velocity = sqrt(2*PROPERTIES.gravity*(inAgent.height - outAgent.height) + inAgent.velocity*inAgent.velocity) * PROPERTIES.friction;
            outAgent.time += PROPERTIES.travelDistance/outAgent.velocity;

            out[outIndex] = outAgent;

        }
        
    }

    //printf("x:%i,y:%i,z:%i  x_t:%i,y_t:%i,z_y:%i\n",x,y,z,x_t,y_t,z_t);


}


void GPU_Util::StepAll(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map){

    //10
    //10
    //10
    for(int x = 0;x<inCount;x++){
        printf("In PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",in[x].positionX,in[x].positionY,in[x].velocity,in[x].height,properties.gravity,properties.friction);
    
    }
    int atp = 50;
    int gridNumber = (int)ceil(sqrt(((float)inCount/(float)atp))/(float)512);
    dim3 DimGrid(gridNumber,gridNumber,1);
    dim3 DimBlock(512,1,1);

    Agent* in_d;
    Agent* out_d;
    Map* map_d;
    printf("PROPERTIES : NUMBER %f\n",properties.numberOfDirectionSpawn);
    //cudaMalloc((void **)&properties_d,sizeof(Properties));
    cudaMalloc((void **)&map_d,sizeof(Map));
    cudaMalloc((void **)&out_d,outCount*sizeof(Agent));
    cudaMalloc((void **)&in_d,inCount*sizeof(Agent));
    cudaMemcpy(map_d,&map,sizeof(Map),cudaMemcpyHostToDevice);
    //cudaMemcpy(properties_d,&properties,sizeof(Properties),cudaMemcpyHostToDevice);
    cudaMemcpy(in_d,in,inCount*sizeof(Agent),cudaMemcpyHostToDevice);
    cudaMemcpyToSymbol(PROPERTIES, &properties, sizeof(Properties));
    GPU_Step<<<DimGrid,DimBlock>>>(in_d,inCount,out_d,outCount,atp,map_d);
    cudaDeviceSynchronize();
    cudaMemcpy(out,out_d,outCount*sizeof(Agent),cudaMemcpyDeviceToHost);
    
    for(int x = 0;x<outCount;x++){
        printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out[x].positionX,out[x].positionY,out[x].velocity,out[x].height,properties.gravity,properties.friction);
    
    }
}



void GPU_Util::Prune(Agent* agents,Agent* out,long count, long amountToPrune){
    long keepAmount = count - amountToPrune;
    ShuffleGPU(agents,count);
    vector<int> good;
    vector<int> bad;
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
