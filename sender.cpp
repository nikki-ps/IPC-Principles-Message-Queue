
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "msg.h"    /* For the message struct */
using namespace std;

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	
	/* We initialize our outFile and then we create a file called "keyfile.txt" that contains the string "Hello World". */
	ofstream outFile;
	outFile.open("keyfile.txt");
	outFile << "Hello world" << endl;
	outFile.close();
	/*We now generate a key named "key" of the type key_t */
	/*We then use the "ftok" function in order to generate the unique key */
	key_t key = ftok("keyfile.txt", 'a');
	/* We use the "shmget" functiion to get the id of the shared memory segment; it returns the shared memory 
	identifier associated with the key parameter for an existing shared memory segment. However if the key parameter 
	does not have a shared memory identifier associated with it and the IPC_CREAT flag is specified in the shmflg parameter 
	a new shared memory segment is created */
	/* If shmget returns a value of -1 it was not succesful and an error is printed */
	if((shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 |IPC_CREAT)) < 0)
	{
		perror("The 'shmget' function was not succesful. Error retrieving shared memory segment.");
		exit(-1);
	}
	/* We use "shmat" attach the shared memory segment identified by shmid to the sharedMemPtr. Since shmaddr is NULL, 
	the system chooses a suitable (unused) page-aligned address to attach the segment. */
	sharedMemPtr = shmat(shmid, (void *) 0, 0);
	/* If shmat assigns a value of -1 to 'sharedMemPtr' it was not succesful and an error is printed */
	if(sharedMemPtr == (char *)(-1))
	{
		perror("The 'shmat' function was not succesful. Error attaching shared memory segment.");
		exit(-1);
	}
	/*We use the "msgget" which initializes a new message queue or returns the message queue identifier, a nonnegative integer.
	It return the message queue ID (msqid) of the queue corresponding to the key argument. */
	/* If msgget assigns a value of -1 to 'msqid' it was not succesful and an error is printed */
	if((msqid = msgget(key, 0666 | IPC_CREAT)) <0)
	{
		perror("The 'msgget' function was not succesful. Error attaching to the message queue.");
		exit(-1);
	}
	/* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
	
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* We use "shmdt" to detach from shared memory */
	/* If shmdt returns a value of -1 it was not succesful and an error is printed */
	cout << endl << "Cleaning up. . ." << endl;
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
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName)
{
	/* Open the file for reading */
	FILE* fp = fopen(fileName, "r");
	

	/* A buffer to store message we will send to the receiver. */
	message sndMsg; 
	sndMsg.mtype = SENDER_DATA_TYPE;
	/* A buffer to store message received from the receiver. */
	message rcvMsg;
	
	/* Was the file open? */
	if(!fp)
	{
		perror("fopen");
		exit(-1);
	}
	
	/* Read the whole file */
	while(!feof(fp))
	{
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory. 
 		 * fread will return how many bytes it has actually read (since the last chunk may be less
 		 * than SHARED_MEMORY_CHUNK_SIZE).
 		 */
		if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
		{
			perror("fread");
			exit(-1);
		}
		/* We use the "msgsnd" function to send a message to the receiver telling him that the data is ready.
		If a value of -1 is returned however the function was not succesful and an error is called.
 		 */
		 if (msgsnd(msqid, &sndMsg , sizeof(sndMsg) - sizeof(long), 0) < 0)
		 {
			perror("The 'msgsnd' function was not succesful. Error sending message to receiver.");
			exit(-1);
		}
		 /* We use the "msgrcv" function to receive the message after the receiver has sent it to us. 
		 If a value of -1 is returned however the function was not succesful and an error is called. */
		 if ( msgrcv(msqid, &rcvMsg, sizeof(rcvMsg) - sizeof(long), RECV_DONE_TYPE, 0) < 0 ) {
			perror("The 'msgrcv' function was not succesful. Error receiving the node from the receiver.");	
			exit(-1);
		}
	}
	 /*Now that we have exited the loop, the file has finished being sent. We now use the "msgsnd" function
	 to tell the receiver that we have nothing more to send. We send the message with the size field set to 0.
	 Again, if a value of -1 is returned the function was not succesful and an error is called. */
	 sndMsg.size = 0;
	  if (msgsnd(msqid, &sndMsg , sizeof(sndMsg) - sizeof(long), 0) < 0)
		 {
			perror("The 'msgsnd' function was not succesful. Error sending message to receiver.");
			exit(-1);
		}	
	/* Close the file */
	fclose(fp);
}
int main(int argc, char** argv)
{
	
	/* Check the command line arguments */
	if(argc < 2)
	{
		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
		exit(-1);
	}
		
	/* Connect to shared memory and the message queue */
	init(shmid, msqid, sharedMemPtr);
	
	/* Send the file */
	send(argv[1]);
	/*Code to print out the resulting file, "recvfile", to show that the message was
	succesfully transferred */
	string rcvText; 
	fstream file; 
	file.open("recvfile"); 
	cout << "Message received; Printing recvfile below..." << endl;
	while(true)
	{ 
		file>> rcvText; 
		if(file.eof())
		{
			break;
		}
		cout<< rcvText << " "; 
	} 
	
	/* Cleanup */
	cleanUp(shmid, msqid, sharedMemPtr);
	return 0;
}
