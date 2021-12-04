#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <CPU_UTIL.h>
void Usage(char* prog_name);

int main(int argc, char* argv[]){

	Agent initial = *new Agent(0,0,0,0,0,nullptr);

	
	CPU_UTIL::AgentStep(initial);

	return 0;



}


void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <number_of_direction_spawn> <friction>\n",prog_name);
	exit(0);
}
