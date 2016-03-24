/*
 * train.h
 *
 * Some definitions for the Trains
 */
#ifndef __TRAIN__H
#define __TRAIN__H

/* Trains at maximum MAX_LENGTH long and minimum MIN_LENGTH long */
#define MIN_LENGTH	3
#define MAX_LENGTH	25
#define MAX_NUM_OF_TRAINS 1000

/* Trains can be headed in one of two directions: EAST or WEST */
#define DIRECTION_NONE	0
#define	DIRECTION_WEST  1 	
#define	DIRECTION_EAST	2	

/* To simulate the length of the train, we will sleep for 
 * length*SLEEP_MULTIPLE when leaving the station and
 * crossing the bridge.
 */
 #define SLEEP_MULTIPLE	100000
 
/*
 * The information about a train.
 */
typedef struct TrainInfo
{
	int	trainId;
	int	direction;
	int	length;
} TrainInfo;

/*
 * Initialize the train library.
 */
void	initTrain ( char *filename );

/*
 * Allocate a new train structure with a new trainId, 
 * trainIds are assigned consecutively, starting at 0
 *
 * Randomly choose a direction and a length.
 */
TrainInfo *createTrain ( void );

#endif


