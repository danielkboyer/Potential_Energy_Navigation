#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <CPU_Util.h>
#include <Agent.h>
#include <Util.h>
#include <FileWriter.h>
void Usage(char* prog_name);

int main(int argc, char* argv[]){

	if(argc != 10)
		Usage(argv[0]);
	//********** Collect arguments **********
	string fileName = argv[1];
	int startingCount = stoi(argv[2]);
	int startingX = stoi(argv[3]);
	int startingY = stoi(argv[4]);
	int numberOfDirectionSpawn = stoi(argv[5]);
	float travelDistance = stof(argv[6]);
	float startVelocity = stof(argv[7]);
	long maxAgentCount = stol(argv[8]);
	float friction = stof(argv[9]);
	//********** END Collect arguments **********

	//Initialize properties
	Properties properties = *new Properties(friction,travelDistance,maxAgentCount,startingX,startingY);

	//Initialize file_Writer
	FileWriter fileWriter = *new FileWriter();
	fileWriter.OpenFile("out_"+fileName);

	//Initialize map
	Map map = *new Map();
	map.ReadFile(fileName);


	Agent* a = new Agent[startingCount];
	long startAgentId = 0;
	float radiusInterval = 2*M_PI/startingCount;
	float currentDirection = 0;
	float startingHeight = map.GetHeight(startingX,startingY);
	for(int x = 0;x<startingCount;x++){
		a[x] = *new Agent(currentDirection,startingX,startingY,startingHeight,startVelocity,0,x,-1,false);
	}
	Util *utility = new CPU_Util();
	long aLength = startingCount;
	while(aLength > 0){

		
		int prunedAmount = 0;	
		if(aLength > maxAgentCount){
			//prune here

		}

		Agent* b = new Agent[aLength - prunedAmount];
		int currentBIndex = 0;
		for(int x = 0;x<aLength;x++){
			if(!a[x].pruned){
				b[currentBIndex++] = *new Agent(a[x]);

			}
		}

		


		//Write to file here
		fileWriter.Write(b,startAgentId);
		for(int x = 0;x<aLength;x++){
			
		}

		


	}
	

	Agent initial = *new Agent(0,0,0,0,0,nullptr);

	
	utility->AgentStep(initial,initial,30,nullptr,nullptr);
	CPU_UTIL::AgentStep(initial);

	return 0;



}


void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y>  <number_of_direction_spawn> <travel_distance> <start_velocity> <max_agent_count> <friction>\n",prog_name);
	exit(0);
}
