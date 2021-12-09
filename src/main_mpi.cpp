#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <mpi.h>

#include "Agent.h"
#include "Util.h"
#include "Serial_Util.h"
#include "vector"

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
    if(agent.pruned == 0)
        copy.pruned = false;
    return copy;
}

int main(int argc, char* argv[]){
    int my_rank, comm_sz;
    double start_1, finish_1, loc_elapsed_1=0, elapsed_1;
    double start_prune, finish_prune, loc_elapsed_prune=0, elapsed_prune;
    double start_step, finish_step, loc_elapsed_step=0, elapsed_step;

    /* Let the system do what it needs to start up MPI */
    MPI_Init(NULL,NULL);
    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    // start total execution time
    MPI_Barrier(MPI_COMM_WORLD);
    start_1 = MPI_Wtime();

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



    
    if (my_rank ==0)
        printf("There are %i ranks \n", comm_sz);
    printf("Hello from rank %i \n", my_rank);

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
    class Agent* a;

    if(my_rank == 0) {
        // Initialize starting agents at starting position serially
        printf("Initializing %ld Starting Agents at (%d,%d), v=%f\n",startingCount,startingX,startingY,startVelocity);
        a = new Agent[startingCount];
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
    float maxDistance;
    float maxDX;
    float maxDY;
	if(my_rank == 0) {
        float maxDistance = 0;
        float maxDX = -1;
        float maxDY = -1;
    }
    int loopAmount = 0;
    long aLength = startingCount;
	int serialPrune = 0;
    int amountToPrune_loc = 0;
    int bLength_loc = 0;
    int extraALength=0;
    int extraAmountToPrune=0;
    int aLength_loc = 0;
    
	MPI_Barrier(MPI_COMM_WORLD);
    while(aLength > 0){
		//Start Loop
        
        // broadcast this variable out at the start of each loop from rank 0
        // int serialPrune_p;
		// MPI_Bcast(&serialPrune_p,1,MPI_INT,0,MPI_COMM_WORLD);
        
        MPI_Bcast(&serialPrune,1,MPI_INT,0,MPI_COMM_WORLD);
        
        MPI_Barrier(MPI_COMM_WORLD);
        printf("serialPrune %i on rank %i \n", serialPrune, my_rank);
        // make local variables 
        long amountToPrune;
        int bLength;
        class Agent* b;
        struct agent_type* b_struct;
        if(my_rank == 0) {
            printf("\nALength: %ld\nLoop Number:%d\n",aLength, loopAmount);
        }
        amountToPrune = max(aLength - maxAgentCount,(long)0);
        bLength = aLength - amountToPrune;
        
        start_prune = MPI_Wtime();

        //******** PRUNING ON THREADS********//
        if (my_rank == 0) {
            if(serialPrune == 0) {
                printf("Amount to prune: %ld\n",amountToPrune);
                // make variables for local computations
                aLength_loc = floor(aLength/comm_sz);
                //printf("aLength_loc = %i \n", aLength_loc);
                amountToPrune_loc = floor(amountToPrune/comm_sz);
                //printf("amountToPrune_loc = %i \n", amountToPrune_loc);
                extraAmountToPrune = amountToPrune - comm_sz*amountToPrune_loc;
                extraALength = aLength - comm_sz*aLength_loc;
                bLength_loc = aLength_loc - amountToPrune_loc;
                //printf("bLength_loc = %i ", bLength_loc);
            }
        }

        // send things to all ranks and then prune as long as we are not doing the serial prune
        if(serialPrune == 0) {
            MPI_Bcast(&aLength_loc,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(&amountToPrune_loc,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(&extraAmountToPrune,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(&extraALength,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Bcast(&bLength_loc,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Barrier(MPI_COMM_WORLD);
            int pruneAmount_l = amountToPrune_loc;
            if(my_rank == comm_sz-1){
                pruneAmount_l = amountToPrune_loc + extraAmountToPrune;
                aLength_loc += extraALength;
            }

            // convert agent to struct for scatter with MPI and scatter
            struct agent_type* a_loc;
            a_loc = new struct agent_type[aLength_loc];
            class Agent* b_agent_loc;
            b_agent_loc = new class Agent[aLength_loc - pruneAmount_l];

            if(my_rank == 0){
                struct agent_type* a_struct = new struct agent_type[aLength];

                for(int x = 0;x<aLength;x++){
                    a_struct[x] = AgentToStruct(a[x]); // a is declared on line 144 with the starting points
                }
                delete[] a;
                int counts[comm_sz];
                int displacements[comm_sz];
                for(int x = 0;x<comm_sz;x++){
                    counts[x] = aLength_loc;
                    displacements[x] = aLength_loc*x;
                }
                counts[comm_sz-1] += extraALength;
                MPI_Scatterv(a_struct,counts,displacements,agent_type,a_loc,aLength_loc,agent_type,0,MPI_COMM_WORLD);
                delete[] a_struct;
            }

            else {
                MPI_Scatterv(NULL,NULL,NULL,agent_type,a_loc,aLength_loc,agent_type,0,MPI_COMM_WORLD);
            }
            
            MPI_Barrier(MPI_COMM_WORLD);
            // convert a_loc from struct to agent for the pruning process          
            class Agent* a_agent_loc = new class Agent[aLength_loc];
            for(int x = 0;x<aLength_loc;x++){
                    a_agent_loc[x] = StructToAgent(a_loc[x]);
            }
            delete[] a_loc;
            
            // Do the actual pruning on each thread on the chunk they have been given
            utility->Prune(a_agent_loc, b_agent_loc, aLength_loc, long(pruneAmount_l));

            // MPI_Barrier(MPI_COMM_WORLD);
            // printf("my rank %i,aLength_loc %i, pruneAmount_l %i,  \n",my_rank,aLength_loc,pruneAmount_l);
            // MPI_Barrier(MPI_COMM_WORLD);

            struct agent_type* b_loc = new struct agent_type[aLength_loc - pruneAmount_l];
            // convert the agent b_loc to struct for gathering with MPI
            for(int x = 0;x<aLength_loc - pruneAmount_l;x++){
                    b_loc[x] = AgentToStruct(b_agent_loc[x]);
            }
            delete[] b_agent_loc;

            // gather the structure data from the pruning process
            if(my_rank == 0){
                b_struct = new struct agent_type[bLength];
                int counts[comm_sz];
                int displacements[comm_sz];
                for(int x = 0;x<comm_sz;x++){
                    counts[x] = bLength_loc;
                    displacements[x] = x*bLength_loc;
                }
                counts[comm_sz-1] += extraALength-extraAmountToPrune;
                MPI_Gatherv(b_loc,aLength_loc-pruneAmount_l,agent_type,b_struct,counts,displacements,agent_type,0,MPI_COMM_WORLD);

                b = new class Agent[bLength];
                // convert b from struct to agent
                for(int x = 0;x<bLength;x++){
                        b[x] = StructToAgent(b_struct[x]);
                }
                delete[] b_struct;
            }
            else{
                MPI_Gatherv(b_loc,aLength_loc-pruneAmount_l,agent_type,NULL,NULL,NULL,agent_type,0,MPI_COMM_WORLD);
            }
            MPI_Barrier(MPI_COMM_WORLD);

		    //******** END PRUNING ON THREADS********//
            delete[] b_loc;
        }
        finish_prune = MPI_Wtime();
        loc_elapsed_prune += finish_prune-start_prune;


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
                b = new class Agent[bLength];
                for(int x = 0;x<bLength;x++){
                    b[x] = a[good[x]];
                }
                if(bLength > maxAgentCount){
                    serialPrune = 0;
                }
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        if(my_rank == 0) {
            printf("Percentage of negative velocities %f\n",b[0].percentage);
            if(b[0].percentage >= ((float)numberOfDirectionSpawn-1)/((float)numberOfDirectionSpawn)){
                serialPrune = 1;
            }
        }
		//******** END PRUNING ********//


		if(my_rank == 0) {
            // Count how mant were pruned
                // This takes just as long as pruning O(n) (minus the computation piece per n)
            for(int x = 0;x<bLength;x++){ 
                // Find which location has the max and what the max is, maybe make a max class
                if(b[x].DistanceFrom(properties.agentStartX,properties.agentStartY) > maxDistance){
                    maxDistance = b[x].DistanceFrom(properties.agentStartX,properties.agentStartY);
                    maxDX = b[x].positionX;
                    maxDY = b[x].positionY;
                }
            }
            printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY); 
        }
        






        //**********START STEP ALL***********//
        start_step = MPI_Wtime();

        // make small agents of size bLength/comm_sz and then have extra for the last one
        bLength_loc = floor(bLength/comm_sz);
        int extraBLength = bLength - bLength_loc*comm_sz;
        aLength = bLength*numberOfDirectionSpawn;
        if(my_rank == comm_sz-1){
            bLength_loc += extraBLength;
        }

        // after declaring blength for all ranks
        aLength_loc = bLength_loc*numberOfDirectionSpawn;

        // convert agent to struct for scatter with MPI
        struct agent_type* b_loc;
        b_loc = new struct agent_type[bLength_loc];                         //struct b_loc
        if(my_rank == 0){
            struct agent_type* b_struct = new struct agent_type[bLength];   // struct b
            for(int x = 0;x<bLength;x++){
                b_struct[x] = AgentToStruct(b[x]);
            }
            delete[] b; // no longer need b
            int counts[comm_sz];
            int displacements[comm_sz];
            for(int x = 0;x<comm_sz;x++){
                counts[x] = bLength_loc;
                displacements[x] = x*bLength_loc;
            }
            counts[comm_sz-1] += extraBLength;

            MPI_Scatterv(b_struct,counts,displacements,agent_type,b_loc,bLength_loc,agent_type,0,MPI_COMM_WORLD);
        }
        else{
            MPI_Scatterv(NULL,NULL,NULL,agent_type,b_loc,bLength_loc,agent_type,0,MPI_COMM_WORLD);
        }
        // convert b_loc from struct to agent
        class Agent* b_agent_loc = new class Agent[bLength_loc];
        for(int x = 0;x<bLength_loc;x++){
                b_agent_loc[x] = StructToAgent(b_loc[x]);
        }
        // make agent a local for output
        class Agent* a_agent_loc;                                           // agent a_loc
        a_agent_loc = new class Agent[aLength_loc];
        delete[] b_loc;

        //Perform the step all in parallell, different for impilimentation
        utility->StepAll(b_agent_loc,bLength_loc,a_agent_loc,aLength_loc,properties,map);

        struct agent_type* a_loc = new struct agent_type[aLength_loc];
        // convert the agent a_loc to struct for gathering with MPI
        for(int x = 0;x<aLength_loc;x++){
                a_loc[x] = AgentToStruct(a_agent_loc[x]);
        }
        delete[] a_agent_loc;
        
        // gather the structure data from the pruning process
        if(my_rank == 0){
            struct agent_type* a_struct = new struct agent_type[aLength];
            int counts[comm_sz];
            int displacements[comm_sz];
            for(int x = 0;x<comm_sz;x++){
                counts[x] = aLength_loc;
                displacements[x] = x*aLength_loc;
            }
            
            counts[comm_sz-1] += extraBLength*numberOfDirectionSpawn;

            MPI_Gatherv(a_loc,aLength_loc,agent_type,a_struct,counts,displacements,agent_type,0,MPI_COMM_WORLD);

            a = new class Agent[aLength];
            // convert a from struct to agent
            for(int x = 0;x<aLength;x++){
                a[x] = StructToAgent(a_struct[x]);
            }
            delete[] a_struct;
        }
        else{
            MPI_Gatherv(a_loc,aLength_loc,agent_type,NULL,NULL,NULL,agent_type,0,MPI_COMM_WORLD);
        }
        finish_step = MPI_Wtime();
        loc_elapsed_step += finish_step-start_step;
        
        //******** END OF STEP ALL************

        // MPI_Barrier(MPI_COMM_WORLD);

        // if (my_rank==0){
        //     for(int x = 0;x<aLength;x++){
        //         printf("END STEP     In PositionX, %f, PositionY %f, Velocity %f, height %f\n",a[x].positionX,a[x].positionY,a[x].velocity,a[x].height);
        //     }
        // }
        MPI_Barrier(MPI_COMM_WORLD);
        // //


        loopAmount++;

	}
    //if (my_rank == 0)
	    //printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);

    MPI_Barrier(MPI_COMM_WORLD);
        // end all timings
    finish_1 = MPI_Wtime();
    loc_elapsed_1 = finish_1-start_1;
    MPI_Reduce(&loc_elapsed_1, &elapsed_1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&loc_elapsed_prune, &elapsed_prune, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&loc_elapsed_step, &elapsed_step, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    // print the timings
    if (my_rank == 0)
        printf("Elapsed total program time = %e\n", elapsed_1);
    if (my_rank == 0)
        printf("Elapsed prune time = %e\n", elapsed_prune);
    if (my_rank == 0)
        printf("Elapsed step comp time = %e\n", elapsed_step);
    MPI_Barrier(MPI_COMM_WORLD);
    /* Shut down MPI */
    MPI_Finalize();
	
    return 0;
}



void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <direction_spawn_radius> <number_of_direction_spawn> <travel_distance> <start_velocity> <max_agent_count> <friction> <run_type>\n",prog_name);
	exit(0);
}
