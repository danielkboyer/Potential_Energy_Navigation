#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <omp.h>

#include "timer.h" // use this for getting times
#include "Agent.h"
#include "Util.h"
#include "FileWriter.h"
#include "Serial_Util.h"
#include "GPU_Util.h"
#include "vector"

void Usage(char* prog_name);

int main(int argc, char* argv[]){

	if(argc < 12 || argc>14)
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


	// get threads for Pragma omp
	int threadCount = 0;
	if (argc==13)
		threadCount = stoi(argv[12]);

	//********** END Collect arguments **********
	double start_total_time=0;
	double elapsed_total_time=0;
	double end_total_time=0;
	GET_TIME(start_total_time);

    double start_stepping_time=0;
    double end_stepping_time=0;
	double elapsed_stepping_time=0;

	double total_prune_time = 0;
	double end_prune_time = 0;
	double prune_start = 0;

	
	printf("Initializing FileWriter\n");
	//Initialize file_Writer
	FileWriter fileWriter = *new FileWriter();
	fileWriter.OpenFile("out_"+fileName);

	printf("Initializing Map\n");
	//Initialize map
	Map map = *new Map();
	map.ReadFile(fileName);

	printf("Initializing Properties\n");
	//Initialize properties
	Properties properties = *new Properties(threadCount,friction,travelDistance,maxAgentCount,startingX*map.GetPointDistance(),startingY*map.GetPointDistance(),numberOfDirectionSpawn,directionSpawnRadius);

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

	// initialize the utility input from the user
	printf("Initializing Utility\n");
	Util *utility;

	if(runType == 0){
		printf("Using Serial\n");
	utility = new Serial_Util();
	}
	else if(runType == 1){
		printf("Using GPU\n");
		utility = new GPU_Util();
	}
	else{	
		printf("invalid Utility should be 0 for serial, 1 for GPU, 2 for MPI");
	}
	
	// initialize variables for possibly parallel computing
	long aLength = startingCount;
	int loopAmount = 0;
	float maxDistance = 0;
	float maxDX = -1;
	float maxDY = -1;
	bool serialPrune = false;
	while(aLength > 0){
		//Start Loop
		printf("\nALength: %ld\nLoop Number:%d\n",aLength,loopAmount);

		//******** PRUNING ********//
		long amountToPrune;
		int bLength;
		Agent* b;
		amountToPrune = max(aLength - maxAgentCount,(long)0);

		GET_TIME(prune_start)
		if(serialPrune == false)
		{
			//printf("Amount to prune: %ld\n",amountToPrune);
			bLength = aLength - amountToPrune;
			b = new Agent[bLength];
			//printf("starting prune amountToPrune: %ld\n",amountToPrune);
			utility->Prune(a,b, aLength, long(amountToPrune), properties);
			
			//printf("Percentage of negative velocities %f\n",b[0].percentage);
			if(b[0].percentage >= ((float)numberOfDirectionSpawn-1)/((float)numberOfDirectionSpawn)){
				serialPrune = true;
			}
		}
		else{

			//printf("Serial prune\n");
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

			if(bLength > maxAgentCount)
			{
				serialPrune = false;
			}
			//printf("B Length Serial %i \n",bLength);

		}
		GET_TIME(end_prune_time);
		total_prune_time += end_prune_time - prune_start;
		//******** END PRUNING ********//
		
		// Count how mant were pruned
			// This takes just as long as pruning O(n) (minus the computation piece per n) 
			// but parallel is also serial from critical section 
		for(int x = 0;x<bLength;x++){ 
			// Find which location has the max and what the max is
			if(b[x].DistanceFrom(properties.agentStartX,properties.agentStartY) > maxDistance){
				maxDistance = float(b[x].DistanceFrom(properties.agentStartX,properties.agentStartY));
				maxDX = float(b[x].positionX);
				maxDY = float(b[x].positionY);
				//printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);
			}
			
		}
		//printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);
		//printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);
		//Write to file here, this is probbaly the same for each implimentation
		fileWriter.Write(b,startAgentId);
		startAgentId += bLength;

		delete[] a;
		aLength = bLength*numberOfDirectionSpawn; 
		a = new Agent[aLength]; // spawn all new agents

		GET_TIME(start_stepping_time);
		//Perform the step all in parallell, different for impilimentation
		utility->StepAll(b,bLength,a,aLength,properties,map);

		// for(int x = 0;x<aLength;x++){
        // 	printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",a[x].positionX,a[x].positionY,a[x].velocity,a[x].height,properties.gravity,properties.friction);
    	// }
		GET_TIME(end_stepping_time);
		elapsed_stepping_time += end_stepping_time-start_stepping_time;
		delete[] b;
		loopAmount++;


	}
	GET_TIME(end_total_time);
	elapsed_total_time = end_total_time-start_total_time;
	printf("Total time  %lf\n", elapsed_total_time);
	printf("Stepping time  %lf\n", elapsed_stepping_time);
	printf("Prune time is %lf\n",total_prune_time) ;
	printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);
	return 0;
}

void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <direction_spawn_radius> <number_of_direction_spawn> <travel_distance> <start_velocity> <max_agent_count> <friction> <run_type>\n",prog_name);
	exit(0);
}
