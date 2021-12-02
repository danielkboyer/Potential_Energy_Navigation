#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <semaphore.h>
const int MAX_THREADS = 64;
sem_t* semaphores;
int** bins;
int thread_count;
int bin_count;
float min_meas;
float max_meas;
int data_count;
//Random numbers
float* rnd_numbers;	

void Usage(char* prog_name);
void *CountNumbers(void* rank);

int main(int argc, char* argv[]){
	long thread;

	pthread_t* thread_handles;
	
	if(argc != 6)
		Usage(argv[0]);
	/* Parse arguments into variables */

	//The number of threads to run the program with
	thread_count = strtol(argv[1], NULL, 10);
	//The number of bins in the array
	bin_count = strtol(argv[2],NULL,10);
	//The smallest number allowed
	min_meas = std::stof(argv[3]);
	//The largest number allowed
	max_meas = std::stof(argv[4]);
	//The amount of data to produce (randomly)
	data_count = strtol(argv[5],NULL,10);
	//The upper bound of each bin
	float bin_maxes [bin_count];
	//The amount in each bin
	int bin_counts [bin_count];
	//Make sure threads are in range
	if(thread_count <=0 || thread_count > MAX_THREADS)
		Usage(argv[0]);
	//allocate the space for the threads
	thread_handles =(pthread_t*) malloc(thread_count*sizeof(pthread_t));
	bins =(int**) malloc((thread_count*bin_count)*sizeof(int));
	semaphores =(sem_t*) malloc(thread_count*sizeof(sem_t));
	rnd_numbers = (float*) malloc(data_count*sizeof(float));
	//Initialize srand to 100
	srand(100);
	//Add numbers to list
	printf("min_meas: %f, max_meas %f \n",min_meas,max_meas);
	for(int i = 0;i<data_count;i++){
		rnd_numbers[i] = ((((float)rand()) / RAND_MAX) * (max_meas - min_meas)) + min_meas;
		//printf("i - %d: %f\n",i,rnd_numbers[i]);
		
	}
	printf("Finished Generating Random Numbers");
	//Create threads and pass them the CountNumbers Function
	for (thread=0;thread<thread_count;thread++){
		pthread_create(&thread_handles[thread],NULL,CountNumbers,(void*) thread);
	}
	for(thread=0; thread< thread_count;thread++){
		pthread_join(thread_handles[thread],NULL);
	}
	free(thread_handles);
	free(bins);
	free(semaphores);

	return 0;



}
void *CountNumbers(void* rank){
	long my_rank = (long) rank;
	//keeps track of how many times this thread has recieved or pushed
	int i = 1;
	//the amount of bins per thread
	float rate = (float)data_count/(float)thread_count;
	//the start index of each thread
	int start_index = my_rank * rate;
	//the end index of each thread
	int end_index = (my_rank+1) * rate;
	//edge case for last thread
       	if(my_rank +1 == thread_count)
		end_index = data_count;
	//the float seperator between bins
	float bin_divider = (max_meas - min_meas)/bin_count;	
	int my_bin [bin_count];
	//printf("starting %ld thread, start=%d, end=%d\n",my_rank,start_index,end_index);	
	//each thread will calculate its local bin here
	for(int x = start_index;x<end_index;x++){
		int bin_place = (int)((rnd_numbers[x]-min_meas)/bin_divider);	
		//printf("bin place for %f is %d, bin_divider=%f\n",rnd_numbers[x],bin_place,bin_divider);
		my_bin[bin_place] +=1;
	}		
	//assign the local bin inside of the bins int pointer 2d array
	bins[my_rank] = my_bin;
	while(true){
		//the number that helps determine where to push or receive from
		int factor = pow(2,i++);
		//In this case, the thread should wait for a receive
		if(my_rank%factor == 0 && my_rank+factor/2 < thread_count){
			sem_wait(&semaphores[my_rank+factor/2]);
			printf("Thread %ld received\n",my_rank);
			for(int i = 0;i<bin_count;i++){
				
				my_bin[i] += bins[my_rank+factor/2][i];
				
			}

				
		}
		//in this case the thread should push to another thread and exit
		else if (my_rank % factor == factor/2){

			printf("Thread %ld pushed\n",my_rank);
			sem_post(&semaphores[my_rank]);
			
			break;	
		}
		//if zero thread reaches here it means the program has completed
		else if(my_rank == 0)
		{
			
			printf("Thread %ld broke\n",my_rank);
			//Print the results
			printf("bin_maxes = ");
			for(int i = 0;i<bin_count;i++)
				printf("%f ",bin_divider * i + min_meas);
			printf("\nbin_counts = ");
			for(int i = 0;i<bin_count;i++)
				printf("%d ",bins[my_rank][i]);
			printf("\n");
			break;
		}
	}
	return NULL;
}

void Usage(char* prog_name){
	fprintf(stderr, "usage: %s <starting_count> <starting_x> <starting_y> <file_name> <friction> <number_of_direction_spawn>\n",prog_name);
	exit(0);
}
