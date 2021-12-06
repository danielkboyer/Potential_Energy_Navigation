#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>

#include "Agent.h"
#include "Util.h"
#include "FileWriter.h"
#include "CPU_Util.h"
void Usage(char* prog_name);

int main(int argc, char* argv[]){

	if(argc != 11)
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
	Properties properties = *new Properties(friction,travelDistance,maxAgentCount,startingX*map.GetPointDistance(),startingY*map.GetPointDistance());

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
	Util *utility = new CPU_Util();
	long aLength = startingCount;

	while(aLength > 0){

		printf("\nALength: %ld\n",aLength);
		long prunedAmount = 0;	
		if(aLength > maxAgentCount){
			printf("ALength %ld>%ld \n",aLength,maxAgentCount);
			// get stats for prunning
			// have barrier for threads and only do on thread 1
			int amountToPrune = aLength - maxAgentCount + 100;
			printf("\namount to prune: %i\n",amountToPrune);
			long sampleRate = 300;
			Stat* stat = new Stat();
			utility->CalcAvg(a, properties, sampleRate, *stat, aLength, long (amountToPrune));
			printf("Stat averages (D_AVG:%f) (E_AVG:%f) (OFFSET:%f)\n",stat->d_avg,stat->E_avg,stat->offset);
			//prune here
			for(int x = 0;x<aLength;x++){
			
				utility->CheckPrune(a[x], properties,*stat);
				// function to count how mant were pruned, this will change for implimentation
				if (a[x].pruned) prunedAmount+=1;
			}
			printf("Pruned Amount %ld\n",prunedAmount);
			delete stat;
		}


		int bLength = aLength - prunedAmount;
		Agent* b = new Agent[bLength];
		int currentBIndex = 0;
		for(int x = 0;x<aLength;x++){
			if(!a[x].pruned){
				b[currentBIndex++] = a[x];
				//b[currentBIndex-1].direction = 1000;
				//printf("a %f, b %f\n",a[x],b[currentBIndex-1]);
			}
		}


		
		//Write to file here
		fileWriter.Write(b,startAgentId);
		startAgentId += bLength;

		delete[] a;
		aLength = bLength*numberOfDirectionSpawn;
		a = new Agent[aLength];
		for(int x = 0;x<bLength;x++){
			
			int aIndex = x*numberOfDirectionSpawn;
			for(int y = 0;y<numberOfDirectionSpawn;y++){
				float newDirection = b[x].direction - directionSpawnRadius/2 + directionSpawnRadius/(numberOfDirectionSpawn-1) * y;
				//a[aIndex+y] = Agent();
				a[aIndex+y] = utility->AgentStep(b[x],newDirection,properties,map);
				
				//printf("Agent position %f,%f\n",a[aIndex+y].positionX,a[aIndex+y].positionY);
			}
			
			
		}
		delete[] b;
		


	}
	
	return 0;



}


void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <direction_spawn_radius> <number_of_direction_spawn> <travel_distance> <start_velocity> <max_agent_count> <friction>\n",prog_name);
	exit(0);
}
