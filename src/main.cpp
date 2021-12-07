#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>

#include "Agent.h"
#include "Util.h"
#include "FileWriter.h"
#include "Serial_Util.h"
#include "GPU_Util.h"
#include "MPI_Util.h"
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
	Properties properties = *new Properties(friction,travelDistance,maxAgentCount,startingX*map.GetPointDistance(),startingY*map.GetPointDistance(),numberOfDirectionSpawn,directionSpawnRadius);

	// Initialize starting agents at starting position serially
	printf("Initializing %ld Starting Agents at (%d,%d), v=%f\n",startingCount,startingX,startingY,startVelocity);
	Agent* a = new Agent[startingCount];
	long startAgentId = 0;
	float radiusInterval = 2*M_PI/startingCount;
	float currentDirection = 0;
	float startingHeight = map.GetHeight(startingX,startingY);
	printf("Starting height: %f\n",startingHeight);

	for(int x = 0;x<startingCount;x++){
		a[x] = Agent(currentDirection,startingX*map.GetPointDistance(),startingY*map.GetPointDistance(),startingHeight,startVelocity,0,x,-1,false);
		currentDirection+= radiusInterval;
	}
	
	// initialize the utility input from the user
	printf("Initializing Utility\n");
	Util *utility;

	if(runType == 0){
	utility = new Serial_Util();
	}
	else if(runType == 1){
		utility = new GPU_Util();
	}
	else if(runType == 2){
		utility = new MPI_Util();
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
	while(aLength > 0){
		//Start Loop
		printf("\nALength: %ld\nLoop Number:%d\n",aLength,loopAmount);
		
	
		//******** PRUNING ********//
		long prunedAmount = 0;	
		long amountToPrune = max(aLength - maxAgentCount,(long)0);
		printf("Amount to prune: %ld\n",amountToPrune);
	
		utility->Prune(a, aLength, long(amountToPrune));
		/// Count up how many were pruned from prunning process
		long prunedAmount_prunning = 0;
		for(int x = 0;x<aLength;x++){ 
			if(a[x].pruned==true) {
				prunedAmount_prunning +=1;
			}
		}
		printf("Pruned Amount from RandPrune %ld\n",prunedAmount_prunning);

		//******** END PRUNING ********//
		
		// Count how mant were pruned, this will change for implimentation
			// This takes just as long as pruning O(n) (minus the computation piece per n)
			// We could do a reduce sum here
		for(int x = 0;x<aLength;x++){ 
			// Find which location has the max and what the max is, maybe make a max class
			if(a[x].DistanceFrom(properties.agentStartX,properties.agentStartY) > maxDistance){
				maxDistance = a[x].DistanceFrom(properties.agentStartX,properties.agentStartY);
				maxDX = a[x].positionX;
				maxDY = a[x].positionY;
			}
			// Count up how many were pruned
			if(a[x].pruned==true) {
				prunedAmount+=1;
			}
		}
		printf("Total Pruned Amount %ld\n",prunedAmount);

		// Make new array without prunned agents
			// This might be able to be optimized, unsure how though
		int bLength = aLength - prunedAmount;
		printf("Amount left %i\n",bLength);
		Agent* b = new Agent[bLength];
		int currentBIndex = 0;
		for(int x = 0;x<aLength;x++){
			if(!a[x].pruned){
				b[currentBIndex++] = a[x];
			}
		}
		
		//Write to file here, this is probbaly the same for each implimentation
		fileWriter.Write(b,startAgentId);
		startAgentId += bLength;

		delete[] a;
		aLength = bLength*numberOfDirectionSpawn; 
		a = new Agent[aLength]; // spawn all new agents

		//Perform the step all in parallell, different for impilimentation
		utility->StepAll(b,bLength,a,aLength,properties,map);
		
		delete[] b;
		loopAmount++;
	}
	
	printf("Max Distance is %f, at (%f,%f)\n",maxDistance,maxDX,maxDY);
	return 0;
}

void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <direction_spawn_radius> <number_of_direction_spawn> <travel_distance> <start_velocity> <max_agent_count> <friction> <run_type>\n",prog_name);
	exit(0);
}
