/*
 * assign2.c
 *
 * Name: Kristy Sebastian
 * ASSIGNMENT 2: TRAIN SIMULATION
 * CSC 360
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <string.h>
#include "train.h"


/* Counts the number of east trains. Used to determine when a waiting west train will pass */
int eastCount = 0;

/* Determines whether the bridge is free */
int bridgeFree = 1;

/* Counts number of trains in trainsWaiting array */
int numOfTrainsWaiting = 0;

pthread_mutex_t request_bridge = PTHREAD_MUTEX_INITIALIZER;

/* Initialize mutex and conditional variable for waiting trains */
pthread_mutex_t criticalSection = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t convar = PTHREAD_COND_INITIALIZER;


/* Struct of arrays for keeping track of waiting trains */
struct TrainInfo* trainsWaiting[MAX_NUM_OF_TRAINS];

void ArriveBridge (TrainInfo *train);
void CrossBridge (TrainInfo *train);
void LeaveBridge (TrainInfo *train);

/*
 * This function is started for each thread created by the
 * main thread.  Each thread is given a TrainInfo structure
 * that specifies information about the train the individual 
 * thread is supposed to simulate.
 */
void * Train ( void *arguments ){
	TrainInfo	*train = (TrainInfo *)arguments;

	/* Sleep to simulate different arrival times */
	usleep (train->length*SLEEP_MULTIPLE);

	ArriveBridge (train);
	CrossBridge  (train);
	LeaveBridge  (train); 

	free (train);
	return NULL;
}

/* Ensures lineup of waiting trains is only east
 * If a west train is detected, return 1
 */
int onlyEastWaiting(){

	int i;
	int westInArray = 0;
	for(i=0; i<numOfTrainsWaiting; i++){
		if(trainsWaiting[i]->direction == 1){
			westInArray = 1;			
		}
	}
	return westInArray;
}

/* Ensures lineup of waiting trains is only west
 * If an east train is detected, return 1
 */
int onlyWestWaiting(){

	int i;
	int eastInArray = 0;
	for(i=0; i<numOfTrainsWaiting; i++){
		if(trainsWaiting[i]->direction == 2){
			eastInArray = 1;			
		}
	}
	return eastInArray;
}

/* Reorders trainsWaiting and places the next train to cross the bridge at the front of the lineup */
void nextTrainToSend(trainDirection){

	int i;
	int locationOfNextTrain = 0;
	int nextTrain = 0;
	struct TrainInfo* waitingTemp[MAX_NUM_OF_TRAINS];

	/* Find first instance of west/east in trainsWaiting
	 * Capture the location of west/east in locationOfNextTrain
	 * Put first instance of west/east at the beginning of the queue
	*/
	for(i=0; i<numOfTrainsWaiting; i++){
		if(trainsWaiting[i]->direction == trainDirection){
			waitingTemp[0] = trainsWaiting[i];
			locationOfNextTrain = i;
			nextTrain = 1;
			break;
		}
	}

	/* Shift array right */
	int offset = 1;
	if(nextTrain == 1){
		for(i=0; i<numOfTrainsWaiting; i++){
			if(i == locationOfNextTrain){
				offset = 0;
			}else{
				waitingTemp[i+offset] = trainsWaiting[i];
			}
		}
	}

	/* Put contents of waitingTemp back into original array */
	if(nextTrain == 1){
		for(i=0; i<numOfTrainsWaiting; i++){
			trainsWaiting[i] = waitingTemp[i];
		}
	}
	
}

/* Simulate arriving at the bridge. */

void ArriveBridge ( TrainInfo *train ){
	printf ("Train %2d arrives going %s\n", train->trainId, 
			(train->direction == DIRECTION_WEST ? "West" : "East"));

	/* If bridge is free and there are no trains waiting, train crosses bridge */
	pthread_mutex_lock(&request_bridge);
	if(bridgeFree == 1 && numOfTrainsWaiting == 0){
		bridgeFree--;
		if(train->direction == DIRECTION_EAST){
			eastCount++;
		}
		pthread_mutex_unlock(&request_bridge);
		return;
	}

	/* Train waiting for bridge is added to a waiting list called trainsWaiting */
	pthread_mutex_lock(&criticalSection);
	trainsWaiting[numOfTrainsWaiting] = train;
	numOfTrainsWaiting++;
	pthread_mutex_unlock(&criticalSection);


	while((train->trainId != trainsWaiting[0]->trainId || bridgeFree != 1)){
		pthread_cond_wait(&convar, &request_bridge);
	}

	pthread_mutex_lock(&criticalSection);
	int i;
	for (i = 0; i < numOfTrainsWaiting; i++){
		trainsWaiting[i] = trainsWaiting[i+1];
	}
	numOfTrainsWaiting--;
	bridgeFree--;
	pthread_mutex_unlock(&criticalSection);

	pthread_mutex_unlock(&request_bridge);

}

/* Simulate crossing the bridge. */

void CrossBridge ( TrainInfo *train ){
	printf ("Train %2d is ON the bridge (%s)\n", train->trainId,
			(train->direction == DIRECTION_WEST ? "West" : "East")); 
	fflush(stdout);
	
	/* 
	 * This sleep statement simulates the time it takes to 
	 * cross the bridge.
	 */
	usleep (train->length*SLEEP_MULTIPLE);

	printf ("Train %2d is OFF the bridge(%s)\n", train->trainId, 
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
}

/* Simulate leaving the bridge */

void LeaveBridge ( TrainInfo *train ){

	/* Alert waiting threads that bridge is free */
	pthread_mutex_lock(&criticalSection);

	/* Sends next train depending on trainsWaiting and number of east trains crossing */
	int onlyEastInArray = onlyEastWaiting();
	int onlyWestInArray = onlyWestWaiting();

	if(numOfTrainsWaiting == 0){
		eastCount = 0;
	}else if(onlyEastInArray == 0){
		/* send east (first entry in the array) */
		eastCount = 1;
	}else if(onlyWestInArray == 0){
		/* send west (first entry in the array) */
		eastCount = 0;
	}else{
		if(eastCount >= 2){
			/* send west */
			nextTrainToSend(DIRECTION_WEST);
			eastCount = 0;
		}else{
			/* send east */
			nextTrainToSend(DIRECTION_EAST);
			eastCount++;
		}
	}

	bridgeFree++;
	pthread_mutex_unlock(&criticalSection);
	pthread_cond_broadcast(&convar);
}

int main ( int argc, char *argv[] ){
	int		trainCount = 0;
	char 		*filename = NULL;

	pthread_t	*tids;
	int		i;

	/* Parse the arguments */
	if ( argc < 2 ){
		printf ("Usage: part1 n {filename}\n\t\tn is number of trains\n");
		printf ("\t\tfilename is input file to use (optional)\n");
		exit(0);
	}
	
	if ( argc >= 2 ){
		trainCount = atoi(argv[1]);
		if(trainCount == 0){
			printf("ERROR - Input for number of trains must be an integer. Please try again.\n");
			exit(0);
		}
		
	}
	if ( argc == 3 ){
		filename = argv[2];
	}	
	

	initTrain(filename);

	tids = (pthread_t *) malloc(sizeof(pthread_t)*trainCount);
	
	/*
	 * Create all the train threads pass them the information about
	 * length and direction as a TrainInfo structure
	 */
	for (i=0;i<trainCount;i++){
		TrainInfo *info = createTrain();
		
		printf ("Train %2d headed %s length is %d\n", info->trainId,
			(info->direction == DIRECTION_WEST ? "West" : "East"),
			info->length );

		if ( pthread_create (&tids[i],0, Train, (void *)info) != 0 ){
			printf ("Failed creation of Train.\n");
			exit(0);
		}
	}

	/* Waits for all train threads to terminate */
	for (i=0;i<trainCount;i++){
		pthread_join (tids[i], NULL);
	}
	
	free(tids);
	return 0;
}

