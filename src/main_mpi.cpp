#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <mpi.h>

#include "Agent.h"
#include "Util.h"
#include "Serial_Util.h"
#include "vector"



// have function that converts agent to struct and then converts struct back to agent
// this will esentially serialize the class and then de serialize it

/* Build a derived datatype for distributing the Agent data */
void Agent_mpi_type(float direction_p, float positionX_p, float positionY_p, float height_p, float velocity_p, float time_p, int Id_p, int parentId_p,float percentage_p,bool pruned_p,MPI_Datatype* Agent_mpi_t_p);


// // create agent class for MPI
// this creates a serialized version of the class that we can then pass to threads
// MPI_Type_struct(count,blocklens,indices,old_types,&new_type);
//     // Create a resized type
//     MPI_Type resized_new_type;
//     MPI_Type_create_resized(new_type,
//                             // lower bound == min(indices) == indices[0]
//                             indices[0],
//                             (MPI_Aint)sizeof(struct Residence),
//                             &resized_new_type);
//     MPI_Type_commit(&resized_new_type);
//     // Free new_type as it is no longer needed
//     MPI_Type_free(&new_type);
//     return resized_new_type;

void Usage(char* prog_name);

struct agent_type
{
    float direction;
        float positionX;
        float positionY;
        float height;
        float velocity;
        float percentage;
        //in seconds
        float time;
        int Id;
        int parentId;
        int pruned;
};


agent_type AgentToStruct(Agent agent){
    agent_type copy;
    copy.direction = agent.direction;
    copy.positionX = agent.positionX;
    copy.positionY = agent.positionY;
    copy.height = agent.height;
    copy.velocity = agent.velocity;
    copy.percentage = agent.percentage;
    copy.time = agent.time;
    copy.Id = agent.Id;
    copy.parentId = agent.parentId;
    copy.pruned = true;
    if(agent.pruned == 0)
        copy.pruned = false;
    return copy;
}

Agent StructToAgent(agent_type agent){
    Agent copy;
    copy.direction = agent.direction;
    copy.positionX = agent.positionX;
    copy.positionY = agent.positionY;
    copy.height = agent.height;
    copy.velocity = agent.velocity;
    copy.percentage = agent.percentage;
    copy.time = agent.time;
    copy.Id = agent.Id;
    copy.parentId = agent.parentId;
    copy.pruned = true;
    if(agent == 0)
        copy.pruned = false;
    return copy;
}
int main(int argc, char* argv[]){

	if(argc != 12)
		Usage(argv[0]);
	//********** Collect arguments **********
	string fileName = argv[1];
	long startingCount = stol(argv[2]);
	int startingX = stoi(argv[3]);
	int startingY = stoi(argv[4]);
	float directionSpawnRadius = stof(argv[5]);
	int numberOfDirectionSpawn = stoi(argv[6]);
	float travelDistance = stof(argv[7]);
	float startVelocity = stof(argv[8]);
	long maxAgentCount = stol(argv[9]);
	float friction = stof(argv[10]);
	int runType = stoi(argv[11]);
	//********** END Collect arguments **********
    int my_rank, comm_sz;
    
    double start_agent_comp, finish_agent_comp, loc_elapsed_agent_comp, agent_comp;
    double start_2, finish_2, loc_elapsed_2, elapsed_2;
    double start_3, finish_3, loc_elapsed_3, elapsed_3;
    /* Let the system do what it needs to start up MPI */
    MPI_Init(NULL,NULL);
    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    //***** INitialize Typees ******//
    MPI_Datatype agent_type;

    int lengths[10] = {1,1,1,1,1,1,1,1,1,1};

    MPI_Aint displacements[10];
    struct agent_type dummy_agent;
    MPI_Aint base_address;
    MPI_Get_address(&dummy_agent, &base_address);
    MPI_Get_address(&dummy_agent.direction, &displacements[0]);
    MPI_Get_address(&dummy_agent.positionX, &displacements[1]);
    MPI_Get_address(&dummy_agent.positionY, &displacements[2]);
    MPI_Get_address(&dummy_agent.height, &displacements[3]);
    MPI_Get_address(&dummy_agent.velocity, &displacements[4]);
    MPI_Get_address(&dummy_agent.percentage, &displacements[5]);
    MPI_Get_address(&dummy_agent.time, &displacements[6]);
    MPI_Get_address(&dummy_agent.Id, &displacements[7]);
    MPI_Get_address(&dummy_agent.parentId, &displacements[8]);
    MPI_Get_address(&dummy_agent.pruned, &displacements[9]);

    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
    displacements[3] = MPI_Aint_diff(displacements[3], base_address);
    displacements[4] = MPI_Aint_diff(displacements[4], base_address);
    displacements[5] = MPI_Aint_diff(displacements[5], base_address);
    displacements[6] = MPI_Aint_diff(displacements[6], base_address);
    displacements[7] = MPI_Aint_diff(displacements[7], base_address);
    displacements[8] = MPI_Aint_diff(displacements[8], base_address);
    displacements[9] = MPI_Aint_diff(displacements[9], base_address);
 
    MPI_Datatype types[10] = { MPI_FLOAT, MPI_FLOAT,MPI_FLOAT,MPI_FLOAT,MPI_FLOAT,MPI_FLOAT,MPI_INT,MPI_INT,MPI_FLOAT, MPI_INT };
    MPI_Type_create_struct(10, lengths, displacements, types, &agent_type);
    MPI_Type_commit(&agent_type);


	printf("Initializing Map\n");
	//Initialize map
	Map map = *new Map();
	map.ReadFile(fileName);

	printf("Initializing Properties\n");
	//Initialize properties
	Properties properties = *new Properties(comm_sz,friction,travelDistance,maxAgentCount,startingX*map.GetPointDistance(),startingY*map.GetPointDistance(),numberOfDirectionSpawn,directionSpawnRadius);
    
    // initialize the utility input from the user
	printf("Initializing Utility\n");
	Serial_Util *utility;
	utility = new Serial_Util();

    if(my_rank == 0) {
        // Initialize starting agents at starting position serially
        printf("Initializing %ld Starting Agents at (%d,%d), v=%f\n",startingCount,startingX,startingY,startVelocity);
        Agent* a = new Agent[startingCount];
        long startAgentId = 0;
        float radiusInterval = 2*M_PI/startingCount;
        float currentDirection = 0;
        float startingHeight = map.GetHeight(startingX,startingY);
        printf("Starting height: %f\n",startingHeight);

        for(int x = 0;x<startingCount;x++){
            a[x] = Agent(currentDirection,startingX*map.GetPointDistance(),startingY*map.GetPointDistance(),startingHeight,startVelocity,0,x,-1,0,false);
            currentDirection+= radiusInterval;
        }
    }
	
	// initialize variables for parallel computing
	if(my_rank == 0) {
        float maxDistance = 0;
        float maxDX = -1;
        float maxDY = -1;
    }
    int loopAmount = 0;
    long aLength = startingCount;
	int serialPrune = 0;
	while(aLength > 0){
		//Start Loop
        
        // broadcast this variable out at the start of each loop from rank 0
		MPI_Bcast(*serialPrune,1,MPI_INT,0,MPI_COMM_WORLD);

        // make local variables 
        if(my_rank == 0) {
            printf("\nALength: %ld\nLoop Number:%d\n",aLength, loopAmount);
            long amountToPrune;
            int bLength;
            Agent* b;
            agent_type* b_struct;
        }

        //******** PRUNING ON THREADS********//
        long amountToPrune_loc = 0;
        int bLength_loc = 0;
        int extraALength;
        int extraAmountToPrune;
        int aLength_loc = 0;
        Agent* b_loc;
        if (my_rank == 0) {
            if(serialPrune == 0) {
                amountToPrune = max(aLength - maxAgentCount,(long)0);
                printf("Amount to prune: %ld\n",amountToPrune);
                bLength = aLength - amountToPrune;
                // make variables for local computations
                aLength_loc = floor(amountToPrune/comm_sz);
                printf("aLength_loc = %i ", aLength_loc);
                amountToPrune_loc = floor(amountToPrune/comm_sz);
                printf("amountToPrune_loc = %i ", amountToPrune_loc);
                extraAmountToPrune = amountToPrune - comm_sz*amountToPrune_loc;
                extraALength = aLength - comm_sz*aLength_loc;
                bLength_loc = aLength_loc - amountToPrune_loc;
                printf("bLength_loc = %i ", bLength_loc);
            }
        }
        // send things to all ranks and then prune as long as we are not doing the serial prune
        if(serialPrune == 0) {
            MPI_Bcast(aLength_loc,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(amountToPrune_loc,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(extraAmountToPrune,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(extraALength,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(bLength_loc,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
            int pruneAmount_l = amountToPrune_loc;
            if(my_rank == comm_sz-1){
                pruneAmount_l = amountToPrune_loc + extraAmountToPrune;
                aLength_loc += extraALength
            }

            // convert agent to struct for scatter with MPI and scatter
            a_loc = new agent_type[alength_loc];
            b_agent_loc = new Agent[aLength_loc - pruneAmount_l];
            if(my_rank == 0){
                agent_type* a_struct = new agent_type[aLength];

                for(int x = 0;x<aLength;x++){
                    a_struct[x] = AgentToStruct(a[x]);
                }
                delete[] a;
                int[comm_sz] counts;
                int[comm_sz] displacements;
                for(int x = 0;x<comm_sz;x++){
                    counts[x] = amountToPrune_loc;
                    displacements[x] = x*amountToPrune_loc;
                }
                counts[comm_sz-1] += extraAmountToPrune;
                MPI_Scatterv(a_struct,counts,displacements,agent_type,a_loc,aLength_loc,agent_type,0,MPI_COMM_WORLD);
            }

            MPI_Scatterv(NULL,NULL,NULL,agent_type,a_loc,aLength_loc,agent_type,0,MPI_COMM_WORLD);

            // convert a_loc from struct to agent for the pruning process          
            Agent* a_agent_loc = new Agent[alength_loc];
            for(int x = 0;x<alength_loc;x++){
                    a_agent_loc[x] = StructToAgent(a_loc[x]);
            }
            delete[] a_loc;
            // Do the actual pruning on each thread on the chunk they have been given
            utility->Prune(a_agent_loc, b_agent_loc, aLength_loc, long(pruneAmount_l));
            
            agent_type* b_loc = new agent_type[aLength_loc - pruneAmount_l];
            // convert the agent b_loc to struct for gathering with MPI
            for(int x = 0;x<aLength_loc - pruneAmount_l;x++){
                    b_loc[x] = AgentToStruct(b_agent_loc[x]);
            }
            delete[] b_agent_loc;

            // gather the structure data from the pruning process
            if(my_rank == 0){
                b_struct = new agent_type[bLength];
                int[comm_sz] counts;
                int[comm_sz] displacements;
                for(int x = 0;x<comm_sz;x++){
                    counts[x] = amountToPrune_loc;
                    displacements[x] = x*amountToPrune_loc;
                }
                counts[comm_sz-1] += extraAmountToPrune;

                MPI_Gatherv(b_loc,aLength_loc-pruneAmount_l,agent_type,b_struct,counts,displacements,agent_type,0,MPI_COMM_WORLD);

                b = new Agent[bLength];
                // convert b from struct to agent
                for(int x = 0;x<bLength;x++){
                        b[x] = StructToAgent(b_struct[x]);
                }
                delete[] b_struct;
            }
            else{
                MPI_Gatherv(b_loc,aLength_loc-pruneAmount_l,agent_type,NULL,NULL,NULL,agent_type,0,MPI_COMM_WORLD);
            }
		    //******** END PRUNING ON THREADS********//
        }
        // serial prune when serial prune is activated by high percentage of 0 velocities
        if(my_rank == 0) {
            if (serialPrune != 0) {
                vector<int> good;
                for(int x = 0;x<aLength;x++){
                    if(a[x].pruned == false){
                        good.push_back(x);
                    }
                }
                bLength = good.size();
                b = new Agent[bLength];
                for(int x = 0;x<bLength;x++){
                    b[x] = a[good[x]];
                }
                if(bLength > maxAgentCount){
                    serialPrune = 0;
                }
            }
        }

        if(my_rank == 0) {
            printf("Percentage of negative velocities %f\n",b[0].percentage);
            if(b[0].percentage >= ((float)numberOfDirectionSpawn-1)/((float)numberOfDirectionSpawn)){
                serialPrune = 1;
            }
        }
		//******** END PRUNING ********//
		
		if(my_rank == 0) {
            // Count how mant were pruned, this will change for implimentation
                // This takes just as long as pruning O(n) (minus the computation piece per n)
                // We could do a reduce sum here
            for(int x = 0;x<bLength;x++){ 
                // Find which location has the max and what the max is, maybe make a max class
                if(b[x].DistanceFrom(properties.agentStartX,properties.agentStartY) > maxDistance){
                    maxDistance = b[x].DistanceFrom(properties.agentStartX,properties.agentStartY);
                    maxDX = b[x].positionX;
                    maxDY = b[x].positionY;
                }
            }
            printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);
        
            aLength = bLength*numberOfDirectionSpawn; 
            a = new Agent[aLength]; // spawn all new agents on rank 0 
        }
        
        // make small agents

        //create struct of new agent
        // send out the structs

        // convert structs to agents for the stepAll function

        //Perform the step all in parallell, different for impilimentation
        utility->StepAll(b_loc,bLength_loc,a,aLength_loc,properties,map);
		
        if(my_rank == 0) {            
            delete[] b;
        }
        loopAmount++;
	}
    if (my_rank == 0)
	    printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);
    /* Shut down MPI */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
	return 0;
}



void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <direction_spawn_radius> <number_of_direction_spawn> <travel_distance> <start_velocity> <max_agent_count> <friction> <run_type>\n",prog_name);
	exit(0);
}
