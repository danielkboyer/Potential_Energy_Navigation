#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>

void Usage(char* prog_name);
void *CountNumbers(void* rank);

int main(int argc, char* argv[]){

	return 0;



}


void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <file_name> <starting_count> <starting_x> <starting_y> <number_of_direction_spawn> <friction>\n",prog_name);
	exit(0);
}
