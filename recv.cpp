#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"    /* For the message struct */
#include <fstream>
#include <iostream>

using namespace std;
/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "recvfile";


/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	// STEP 1: Create file called keyfile.txt, containing "Hello World" (we did so manually)
	ofstream outFile;
	outFile.open("keyfile.txt");
	outFile << "Hello world" << endl;
	outFile.close();
	// STEP 2: Use ftok("keyfile.txt", 'a') in order to generate the key.
	key_t key = ftok("keyfile.txt", 'a');

	//Use shmget to return an identifier to shmid, this will be used for message queue
	if((shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 |IPC_CREAT)) < 0)
	{
		//if we reach here, shmget has failed to execute, exit program
		perror("recv.cpp shmget failed");
		exit(-1);
	}


	//Attach msqid to the message queue using msgget
	//msgget returns message queue identifier
	if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0)
	{
		//if we reach here, msgget has failed, exit program
		perror("recv.cpp msgget failed");
		exit(-1);
	}


	//Attach sharedMemPtr to the shared memory segment using shmat
	sharedMemPtr = shmat(shmid, (void *)0, 0);
	if(sharedMemPtr  == (char *)(-1))
	{
		//if we reach here, shmat has failed, exit program
		perror("recv.cpp shmat failed");
		exit(-1);
	}
}
 

/**
 * The main loop
 */
void mainLoop()
{
	/* The size of the mesage */
	int msgSize;

	//declare var to hold msg
	message msg;
	/* Open the file for writing */
	FILE* fp = fopen(recvFileName, "w");
		
	/* Error checks */
	if(!fp)
	{
		perror("recv.cpp fopen failed");
		exit(1);
	}	
	/* Keep receiving until the sender set the size to 0, indicating that
 	 * there is no more data to send
 	 */
	//Use msgrcv to receive msg from msqid, then send it to address of msg
	//message size will be equal to sizeof(msg)-sizeof(long)
	//with SENDER_DATA_TYPE being the default message type
	if(msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), SENDER_DATA_TYPE,0) < 0)
	{
		//if we reach here, msgrcv failed, close fstream and exit
		fclose(fp);
		perror("recv.cpp msgrcv failed1");
		exit(-1);
	}

	//assign value of msg.size to msgSize
	msgSize = msg.size;


	while(msgSize != 0)
	{	
		/* If the sender is not telling us that we are done, then get to work */
		if(msgSize != 0)
		{
			/* Save the shared memory to file */
			if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("recv.cpp fwrite failed");
			}
			
			//set message to RECV_DONE_TYPE to send message that we are ready
			msg.mtype = RECV_DONE_TYPE;


			//use msgsnd to send message to indicate we are ready for next file chunk
			if(msgsnd(msqid, &msg, 0, 0) < 0)
			{
				//if we reach here, msgsnd has failed
				perror("recv.cpp msgsnd failed");
			}


			//We are ready to receive next message chunk, use msgrcv again
       			if(msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), SENDER_DATA_TYPE,0) < 0)
       			{
               			 //if we reach here, msgrcv failed, close fstream and exit
               			 fclose(fp);
               			 perror("recv.cpp msgrcv failed2");
               			 exit(-1);
		        }
			//assign new msg.size to msgSize
			msgSize = msg.size;
		}
		/* We are done */

		else
		{
			/* Close the file */
			fclose(fp);
		}
	}
}

/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */


void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* We use "shmdt" to detach from shared memory */
	/* If shmdt returns a value of -1 it was not succesful and an error is printed */
	cout << "Cleaning up. . ." << endl;
	if(shmdt(sharedMemPtr) < 0)
	{
		perror("The 'shmdt' function was not succesful. Error detaching from shared memory.");
		exit(-1);
	}
	/*We use "shmctl" destroy the shared memory */
	/* If "shmctl" returns a value of -1 it was not succesful and an error is printed */
	if(shmctl(shmid,IPC_RMID,NULL) < 0)
	{
		perror("The 'shmctl' function was not succesful. Error destroying shared memory.");
		exit(-1);
	}
	/*We use "msgctl" destroy the message queue */
	/* If "msgctl" returns a value of -1 it was not succesful and an error is printed */
	if(msgctl(msqid, IPC_RMID, NULL) < 0 )
	{
		perror("The 'shmctl' function was not succesful. Error destroying shared memory.");
		exit(-1);
	}
	cout << "Success!" << endl;
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{
	/*We installed a singnal handler in case the user presses Ctrl-c, the program deletes the message queues
	and shared memory before exiting. The cleaning functionality is added in ctrlCSignal(); */
	signal(SIGINT, ctrlCSignal);			
	/* Initialize the program */
	init(shmid, msqid, sharedMemPtr);
	
	/* Go to the main loop */
	mainLoop();

	/* "cleanUp(shmid, msqid, sharedMemPtr);" would be the cleanUp function we would call, but it is called
	in sender.cpp already, so calling it again would be redundent" */
	return 0;
}