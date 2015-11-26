//Backend

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

typedef struct Buffer
{
	char message[250];
	int timestamp;
	struct Buffer *next;
}Buffer;

typedef struct ClientStruct
{
	int id;
	char name[15];
	char ip[20];
	int port;
	int timestamp;
	char value[5];
}client;//[3];

client cs;
typedef struct currRecord
{
	int Flag; //0 = Door, 1 = MotionSensor, 2 = kEYCHAIN, 3 = security 
	int timestamp;
}rec;
rec r[4];

int doorFlag;
int motionFlag;
int keychainFlag;
int keychainchange;
int sockfd, gatewaysockfd, c;

//Check if user or thief has entered or left 
char * check() 
{
	char entry[200];
	//if(doorFlag && motionFlag && keychainFlag)
	//{
		if(r[0].timestamp < r[2].timestamp)
		{
			if(r[2].Flag)
				sprintf(entry, "\nUser has come home.........\n"); 
		
			if(!r[2].Flag)
				sprintf(entry, "\nUser has gone out..........\n"); 
		}
			
		if(r[0].Flag && !r[2].Flag)
		{
			if(keychainchange)
			if(r[1].timestamp > r[0].timestamp)
				sprintf(entry, "\nThief has entered house!!!!\n"); 
		}
		/*
		if(r[1].Flag && !r[2].Flag)
			printf("\nUnknown person in house!!!! d%d k%d \n",r[0].Flag, r[2].Flag); */
	//}
	keychainchange = 0;
	if(entry != NULL)
		puts(entry);
	return entry;
}

FILE * fpOutputFile;
//char* file1 = argv[1];
void writeToFile(char * fileData)
{
	fpOutputFile = fopen("PersistentStorage.txt", "a");
	fprintf(fpOutputFile, "%s\n", fileData);
	fclose(fpOutputFile);
}

int main(int argc, char* argv[])
{
	struct sockaddr_in gate, backend;
	
	//Database
	//FILE *fp = fopen("PersistentStorage.txt", "w");	
	
	//Create Socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("\nCould not create Backend socket");
	}
	puts("\nBackend socket is created");	
	
	backend.sin_family = AF_INET;
	backend.sin_addr.s_addr = INADDR_ANY;
	backend.sin_port = htons( 8081 );
	
	//Bind the socket and server address
	if((bind(sockfd, (struct sockaddr *) &backend, sizeof(backend))) < 0)
	{
		perror("\nUnable to Bind socket and address");
		return 1;
	}	
	puts("Socket and address binded");
	puts("Waiting for registrations. . .\n");

	//Listen for connections
	listen(sockfd, 5);
	
	c = sizeof(struct sockaddr_in);
	while (gatewaysockfd = accept(sockfd,(struct sockaddr *) &gate, (socklen_t*)&c))
	{
		char readmsg[200];
		char entry[200];
		char temporary[200];
		size_t msglen;
		//memset(readmsg, 0, 200);
		while((msglen = recv(gatewaysockfd, readmsg, 200, 0)) > 0 )
		{
			//fflush(fp);
			readmsg[msglen] = '\0';
			printf("<<<<<<<Received Message: %s>>>>>>\n", readmsg);
			
			writeToFile(readmsg);  
			
			//puts(readmsg);
			/*
			char * t;
			
			t = strtok(readmsg, ";");
			cs.id =	atoi(t); 
			strcpy(cs.name ,strtok(NULL, ";"));
			strcpy(cs.ip ,strtok(NULL,";"));
			t = strtok(NULL,";");
			cs.port = atoi(t);
			t = strtok(NULL,";");
			cs.timestamp = atoi(t);
			strcpy(cs.value, strtok(NULL,"\n"));
			memset(temporary, 0, 200);
			char value[5];
			sprintf(temporary, "%d--%s--%s--%d--%d--%s", cs.id, cs.name, cs.ip, cs.port, cs.timestamp, cs.value);*/
			//writeToFile(temporary);
			
			/*
			int switcher;
			
			if(strcmp(cs.name, "door") == 0)
				switcher = 0;
			else if (strcmp(cs.name, "motionsensor") == 0)
				switcher = 1;
			else if (strcmp(cs.name, "keychain") == 0)
				switcher = 2;
			
			
			switch(switcher)
			{
				case 0:
				if(doorFlag == 0)
					doorFlag++;
					if(strcmp(cs.value, "Open") == 0)
						r[0].Flag = 1;
					else
					{
						r[0].Flag = 0;
						strcpy(entry, check());
					}
		
				r[0].timestamp = cs.timestamp;
				break;
				
				case 1:
				if(motionFlag == 0)
					motionFlag++;
					if(strcmp(cs.value, "True") == 0)
						r[1].Flag = 1;
					else
						r[1].Flag = 0;
				
				r[1].timestamp = cs.timestamp;		
				break;
				
				case 2:
				if(keychainFlag == 0)
					keychainFlag++;
					if(strcmp(cs.value, "True") == 0)
					{
						if(r[2].Flag == 0)
						{
							r[2].Flag = 1;
							strcpy(entry, check());
							keychainchange=1;
						}
						r[2].Flag = 1;
					}
					else
					{
						if(r[2].Flag == 1)
						{
							r[2].Flag = 0;
							strcpy(entry, check());
							keychainchange=1;
						}
						r[2].Flag = 0;
					}
				r[2].timestamp = cs.timestamp;
				break;
			}	
			if(entry != NULL)
				writeToFile(entry);
			*/
			memset(readmsg, 0, 2000);	
		}
	}	
}
