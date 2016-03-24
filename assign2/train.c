/*
 * train.c
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "train.h"
 

/* A global to assign IDs to our trains */ 
int idNumber = 0;

/* If this value is set to 1, trains lengths
 * etc will be generated randomly.
 * 
 * If it is set to 0, the lengths etc will be
 * input from a file.
 */
int doRandom = 0;

/* The file to input train data from */
FILE *inputFile;

/* No more than 80 characters will be read from an input file */
#define MAXLINE		80

void	initTrain ( char *filename ){
	doRandom = 0;
	
	/* If no filename is specified, generate randomly */
	if ( !filename ){
		doRandom = 1;
		srandom(getpid());
	}else{
		printf("filename: %s \n", filename);
		inputFile = fopen(filename, "r");
		if(inputFile == NULL){
			printf ("File input not implemented.\n");
		}
	}
}
 
/*
 * Allocate a new train structure with a new trainId, trainIds are
 * assigned consecutively, starting at 0
 *
 * Either randomly create the train structures or read them from a file
 */
TrainInfo *createTrain ( void ){
	TrainInfo *info = (TrainInfo *)malloc(sizeof(TrainInfo));

	if (!doRandom){

		char *lineFromFile = malloc(sizeof(char)*80);
		char *directionFromFile = malloc(sizeof(char)*80);
		int lengthFromFile;
		int directionNum = 0;

		while(fgets(lineFromFile, sizeof(lineFromFile), inputFile)){

			/* Convert direction character to string */
			strncpy(directionFromFile, lineFromFile, 1);
			directionFromFile[1] = '\0';

			/* Set direction number for East or West */
			if(directionFromFile[0] == 'W'){
				directionNum = 1;
			}else if(directionFromFile[0] == 'w'){
				directionNum = 1;
			}else if(directionFromFile[0] == 'E'){
				directionNum = 2;
			}else if(directionFromFile[0] == 'e'){
				directionNum = 2;
			}

			/* Put length of train in variable and convert to integer */
			memmove(lineFromFile, lineFromFile+1, strlen(lineFromFile));
			lengthFromFile = atoi(lineFromFile);

			info->trainId = idNumber++;
			info->arrival = 0;
			info->direction = directionNum;
			info->length = lengthFromFile;

			return info;
		}

		fclose(inputFile);

	}else{
		/* Random values assigned in case there is an issue with the input file */	 
		info->trainId = idNumber++;
		info->arrival = 0;
		info->direction = (random() % 2 + 1);
		info->length = (random() % MAX_LENGTH) + MIN_LENGTH;
	}
	return info;
}




