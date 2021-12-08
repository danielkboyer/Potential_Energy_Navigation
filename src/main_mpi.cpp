#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <mpi.h>

#include "Agent.h"
#include "Util.h"
#include "Serial_Util.h"
#include "vector"

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

    /* Let the system do what it needs to start up MPI */
    MPI_Init(NULL,NULL);
    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	printf("Initializing Map\n");
	//Initialize map
	Map map = *new Map();
	map.ReadFile(fileName);

	printf("Initializing Properties\n");
	//Initialize properties
	Properties properties = *new Properties(friction,travelDistance,maxAgentCount,startingX*map.GetPointDistance(),startingY*map.GetPointDistance(),numberOfDirectionSpawn,directionSpawnRadius);
    
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
		MPI_Bcast(serialPrune,1,MPI_INT,0,MPI_COMM_WORLD);

        // make local variables 
        if(my_rank == 0) {
            printf("\nALength: %ld\nLoop Number:%d\n",aLength, loopAmount);
            long amountToPrune;
            int bLength;
            Agent* b;
        }

        //******** PRUNING ON THREADS********//
        long amountToPrune_loc = 0;
        int bLength_loc = 0;
        Agent* b_loc;
        if (my_rank == 0) {
            if(serialPrune == 0) {
                amountToPrune = max(aLength - maxAgentCount,(long)0);
                printf("Amount to prune: %ld\n",amountToPrune);
                bLength = aLength - amountToPrune;
                b = new Agent[bLength];
                // make variables for local computations
                aLength_loc = floor(amountToPrune/comm_sz);
                amountToPrune_loc = floor(amountToPrune/comm_sz);
                int extraAmountToPrune = amountToPrune - comm_sz*amountToPrune_loc;
                int extraALength = aLength - comm_sz*aLength_loc;
            }

        }
        // send things to all ranks and then prune
        MPI_Bcast(aLength_loc,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(amountToPrune_loc,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(extraAmountToPrune,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(extraALength,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        if (my_rank != (comm_sz-1)){
            memcpy(a_loc, a[aLength_loc*my_rank], aLength_loc);
            b_loc = new Agent[aLength_loc-amountToPrune_loc];
            memcpy( void* dest, const void* src, std::size_t count );
        }
        // make data with extras
        else {
            memcpy(a_loc, a[aLength_loc*my_rank], aLength_loc + extraALength);
            b_loc = new Agent[aLength_loc-amountToPrune_loc];
        }
		//******** END PRUNING ON THREADS********//

        utility->Prune(a_loc, b_loc, aLength_loc, long(amountToPrune_loc));
        
        // combine all the  back to rank 0 (a, b)
        if(my_rank == 0) {
            printf("Percentage of negative velocities %f\n",b[0].percentage);
            if(b[0].percentage >= ((float)numberOfDirectionSpawn-1)/((float)numberOfDirectionSpawn)){
                serialPrune = 1;
            }
        }

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
            //Write to file here, this is probbaly the same for each implimentation
            startAgentId += bLength;

            delete[] a;
            aLength = bLength*numberOfDirectionSpawn; 
            a = new Agent[aLength]; // spawn all new agents
        }
        
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
