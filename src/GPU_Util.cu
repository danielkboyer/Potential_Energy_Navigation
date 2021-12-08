#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "GPU_Util.h"
#include "random"


__global__
void GPU_Step(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map);
int intRandGPU(const int & min, const int & max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}
void SwapValueGPU(Agent &a, Agent &b) {
   Agent t = a;
   a = b;
   b = t;
}

void ShuffleGPU(Agent* agents, int count){

    for(int x = 0;x<count;x++){
        int index1 = intRandGPU(0,count-1);
        int index2 = intRandGPU(0,count-1);
        SwapValueGPU(agents[index1],agents[index2]);

    }

}

__global__ void GPU_Step(Agent* in, int inCount, Agent* out, int outCount, Properties* properties, Map* map){
    int x = blockDim.x;
    int y = blockDim.y;
    int z = blockDim.z;

    int x_b = blockIdx.x;
    int y_b = blockIdx.y;
    int z_b = blockIdx.z;
    
    int x_t = threadIdx.x;
    int y_t = threadIdx.y;
    int z_t = threadIdx.z;

    //int mainId = 
    //printf("x:%i,y:%i,z:%i  x_t:%i,y_t:%i,z_y:%i\n",x,y,z,x_t,y_t,z_t);


}
void GPU_Util::StepAll(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map){

    //10
    //10
    //10
    printf("TETSSTSTS hh\n");
    dim3 DimGrid(1,1,1);
    dim3 DimBlock(32,1,1);

    Agent* in_d;
    Agent* out_d;
    Properties* properties_d;
    Map* map_d;
    cudaMalloc((void **)&properties_d,sizeof(Properties));
    cudaMalloc((void **)&map_d,sizeof(Map));
    cudaMalloc((void **)&out_d,outCount*sizeof(Agent));
    cudaMalloc((void **)&in_d,inCount*sizeof(Agent));
    cudaMemcpy(map_d,&map,sizeof(Map),cudaMemcpyHostToDevice);
    cudaMemcpy(properties_d,&properties,sizeof(Properties),cudaMemcpyHostToDevice);
    cudaMemcpy(in_d,in,inCount*sizeof(Agent),cudaMemcpyHostToDevice);

    GPU_Step<<<DimGrid,DimBlock>>>(in_d,inCount,out_d,outCount,properties_d,map_d);

    cudaDeviceSynchronize();
    printf("Cuda done\n");
    for(int x = 0;x<inCount;x++){
		int aIndex = x*properties.numberOfDirectionSpawn;
		for(int y = 0;y<properties.numberOfDirectionSpawn;y++){
			float newDirection = in[x].direction - properties.directionSpawnRadius/2 + properties.directionSpawnRadius/(properties.numberOfDirectionSpawn-1) * y;
			//a[aIndex+y] = Agent();
			out[aIndex+y] = AgentStep(in[x],newDirection,properties,map);		
			//printf("Agent position %f,%f\n",a[aIndex+y].positionX,a[aIndex+y].positionY);
		}
	}
}



void GPU_Util::Prune(Agent* agents,Agent* out,long count, long amountToPrune){
    srand (100);
    long x = 0;
    ShuffleGPU(agents,count);
    for(int i=0;i<count;i++){
        agents[x].pruned = true;
        //printf("ID pruned: %ld\n", x);
    }
}




//Must have a non null out agent

__device__ Agent GPU_Util::AgentStep(Agent in, float newDirection, Properties properties, Map map){
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

__device__ Agent GPU_Util::AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map){  
    out.positionX = in.positionX + cos(newDirection) * properties.travelDistance;
    out.positionY = in.positionY + sin(newDirection) * properties.travelDistance;
    //printf("Agent position %f,%f\n",out->positionX,out->positionY);
    out.direction = newDirection;
    return out;
    
}
//must have out positionX and positionY populated

__device__ Agent GPU_Util::AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map){
    
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
