/*
----ASSIGNMENT 1: SHELL SIMULATION----
 KRISTY SEBASTIAN
 JANUARY 28, 2016
 CSC 360
 -------------------------------------
 */

 
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


struct backgroundList{
	pid_t pidNum;
	char *status;
};

//declaring array of structs for list of background jobs
struct backgroundList jobs[5];

//adds a new job to the background list
void addbackgroundProcess(pid_t pid){
 	
	int i;
	for(i=0; i<5; i++){
		if(jobs[i].pidNum == 0){
			jobs[i].pidNum = pid;
			jobs[i].status = "R";
			break;
		}
	}
}

//removes a terminated job from the background list
 void removeBackgroundProcess(pid_t pid){

 	int i;
	for(i=0; i<5; i++){
		if(jobs[i].pidNum == pid){
			jobs[i].pidNum = NULL;
			jobs[i].status = NULL;
			break;
		}
	}
 }

//waits for a signal that a background child process has terminated
//alerts the user that background process with a specific PID has terminated
//goes to removeBackgroundProcess function to remove terminated process
static void sigchld_hdl (int sig){

	int status;	
	pid_t pid;
    	
	pid = waitpid(-1, &status, WNOHANG);
		
	if(pid > 0) {
		printf("The background process with PID %d has been terminated!\n", pid);
		removeBackgroundProcess(pid); 
	}
			
}

//prints the list of background processes
//counts each job and prints the number of currently running background jobs
void printBglist(){

 	printf("***Background List:***\n");
 	printf("PID   STATUS\n");

 	int i;
 	int jobCount = 0;
	for(i=0; i<5; i++){
		if(jobs[i].pidNum != NULL){
			printf("%d   ", jobs[i].pidNum);
			printf("%s\n", jobs[i].status);
			jobCount++;
		}
	}
 	printf("\nTotal Background jobs: %d\n", jobCount);
}

//handles the command bgkill to kill a background process with the given PID
void executeBgKill(char *command, char *argv[]){

	pid_t killPid = atoi(argv[1]);

	int retVal = kill(killPid, SIGTERM);
		if(retVal < 0){
			printf("error killing process");
		}

	return;
}

//executes a background command
void executeBg(char *command, char *argv[]){

	pid_t pid;

	struct sigaction act;
	memset (&act, '\0', sizeof(act));
	act.sa_handler = sigchld_hdl;
 
	if (sigaction(SIGCHLD, &act, 0)) {
		perror ("sigaction");
	}

	if((pid = fork()) < 0){

		printf("Forking process failed\n");
		exit(1);

	}else if (pid == 0){

		command = argv[1];

		if(execvp(command, argv) < 0){
			printf("ERROR exec failed\n");
			exit(0);				
		} 

	}else{

		addbackgroundProcess(pid);
	}
}

void getDirectory(char *argv[], int tokenCount){

	char *directoryPath = "";
	directoryPath = argv[1];

	if(strcmp(directoryPath, "") != 0){
		int success = chdir(directoryPath);
		if(success != 0){
			printf("directory does not exist");
		}
	}
}

void stopJob(char *command, char *argv[]){

	int i;
	pid_t stopPid = atoi(argv[1]);

	//check if job is already stopped. send error msg if so
	for(i=0; i<5; i++){
		if(jobs[i].pidNum == stopPid && jobs[i].status == "S"){
			printf("ERROR - Process with PID %d is already stopped", stopPid);
			break;
		}
	}

	//stop job with given stopPid
	int retVal = kill(stopPid, SIGSTOP);
		if(retVal < 0){
			printf("error stopping process");
		}

	//change job status to S
	for(i=0; i<5; i++){
		if(jobs[i].pidNum == stopPid){
			jobs[i].status = "S";
			break;
		}
	}

	return;
}

void resumeJob(char *command, char *argv[]){

	int i;
	pid_t resumePid = atoi(argv[1]);

	//check if job has already resumed. send error msg if so
	for(i=0; i<5; i++){
		if(jobs[i].pidNum == resumePid && jobs[i].status == "R"){
			printf("ERROR - Process with PID %d has already resumed", resumePid);
			break;
		}
	}

	//resume job with given resumePid
	int retVal = kill(resumePid, SIGCONT);
		if(retVal < 0){
			printf("error resuming process");
		}

	//change job status to R
	printf("%d", resumePid);
	for(i=0; i<5; i++){
		if(jobs[i].pidNum == resumePid){
			jobs[i].status = "R";
			break;
		}
	}

	return;
}

void executeCommand(char *command, char *argv[]){

	pid_t pid;
	int status;

	if((pid = fork()) < 0){

		printf("Forking process failed\n");
		exit(1);

	}else if (pid == 0){

		if(execvp(command, argv) < 0){
			printf("ERROR exec failed\n");
			exit(0);				
		} 

	}else{

		while (wait(&status) != pid);

	}
}


void interpretCmd(char *cmd){

	char *command = "";
	const char delimiter[2] = " ";
	char *token = strtok(cmd, delimiter);
	char *tempArray[16];

	//for each token, set dynamic size of temp array and insert each token into array
	int tokenCount = 0;
	while(token != NULL){
		tempArray[tokenCount] = malloc(sizeof(char)*50);
		tempArray[tokenCount] = token;
		token = strtok(NULL, delimiter);
		tokenCount++;
	}
	char *argv[tokenCount+1];
	argv[tokenCount] = NULL;

	//the number of tokens is now known so an array called argv containing each command line argument is created
	int i;
	for(i=0; i<tokenCount; i++){
		argv[i] = malloc(sizeof(char)*50);
		argv[i] = tempArray[i];
		printf("[%s]", argv[i]);
	}
	printf("\n");
	command = argv[0];

	if(strncmp(command, "bglist", 6) == 0){
		printBglist();

	}else if(strncmp(command, "bgkill", 6) == 0){
		executeBgKill(command, argv);

	}else if(strncmp(command, "bg", 2) == 0){
		executeBg(command, argv);

	}else if(strncmp(command, "cd", 2) == 0){
		getDirectory(argv, tokenCount);

	}else if(strncmp(command, "stop", 4) == 0){
		stopJob(command, argv);

	}else if(strncmp(command, "start", 5) == 0){
		resumeJob(command, argv);

	}else{
		executeCommand(command, argv);
	}

	return;
}


int main (void){

	for (;;) {
		char maxPathLength[255]; 
		char *currPath = getcwd(maxPathLength, 255);
		printf("%s", currPath);

		char *cmd = readline (">");

		if(strcmp(cmd, "") == 0){
			//free(cmd);
			//continue;
		}else if(strcmp(cmd, "exit") == 0){
			exit(0);
		}else{
			interpretCmd(cmd);
		}
		free (cmd);
	}   
}