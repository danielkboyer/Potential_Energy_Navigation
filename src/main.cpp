#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>

#include "Agent.h"
#include "Util.h"
#include "FileWriter.h"
#include "Serial_Util.h"
#include "GPU_Util.h"
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
	
	printf("Initializing Utility\n");
	Util *utility;
	if(runType == 0){
	utility = new Serial_Util();
	}
	else if(runType == 1){
		utility = new GPU_Util();
	}
	else{
		
	}
	long aLength = startingCount;

	int loopAmount = 0;
	while(aLength > 0){

		
		printf("\nALength: %ld\n",aLength);
		long prunedAmount = 0;	
		if(aLength > maxAgentCount){
			printf("ALength %ld>%ld ",aLength,maxAgentCount);
			// get stats for prunning
			// have barrier for threads and only do on thread 1
			int amountToPrune = aLength - maxAgentCount + maxAgentCount*.05 + 1000;
			printf("\namount to prune: %i\n",amountToPrune);
			long sampleRate = 1000;
			Stat* stat = new Stat();
			utility->CalcAvg(a, properties, sampleRate, stat, aLength, long (amountToPrune));
			printf("Stat averages (D_AVG:%f) (E_AVG:%f) (OFFSET:%f)\n",stat->d_avg,stat->E_avg,stat->offset);
			//perform the prune here in parallell
			utility->Prune(a,aLength,properties,*stat);
			delete stat;
		}

		// function to count how mant were pruned, this will change for implimentation
		//This takes just as long as pruning O(n) (minus the computation piece per n)
		//We could do a reduce sum here
		for(int x = 0;x<aLength;x++){ 
			if(a[x].pruned==true) {prunedAmount+=1;
			}
		}
		printf("Total Pruned Amount %ld\n",prunedAmount);

		//This might be able to be optimized, but at the moment I really don't know
		int bLength = aLength - prunedAmount;
		printf("Amount left %i",bLength);
		Agent* b = new Agent[bLength];
		int currentBIndex = 0;
		for(int x = 0;x<aLength;x++){
			if(!a[x].pruned){
				b[currentBIndex++] = a[x];
			}
		}
		

		
		//Write to file here
		fileWriter.Write(b,startAgentId);
		startAgentId += bLength;


		delete[] a;
		aLength = bLength*numberOfDirectionSpawn;
		a = new Agent[aLength];

		//Perform the step all in parallell
		utility->StepAll(b,bLength,a,aLength,properties,map);
		
		
		delete[] b;
		loopAmount++;


	}
	
	return 0;



}


void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <direction_spawn_radius> <number_of_direction_spawn> <travel_distance> <start_velocity> <max_agent_count> <friction> <run_type>\n",prog_name);
	exit(0);
}
