//Server Program

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <pthread.h>

void * connection_handler(void *);
void * threadReadFun(void *);

typedef struct
{
	char name[15];
	char ip[20];
	int port;
	int value;
	int registered;
	int sock;
	int idNum;
}clientStruct1;

typedef struct
{
	char name[15];
	char ip[20];
	int port;
	int value;
	int registered;
	int sock;
	int idNum;
}clientStruct;
static clientStruct csArr[15] = {0};

int id = -1;
int reg = -1;
char registerIds[300];
int setReg = 1;

int backsockfd;
int vectorClock[4] = {0};
FILE *fpOutputFile;
char *file3;	

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexInner = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{	
	file3 = "GatewayOutputFile.txt";
	
	int sockfd,clientsockfd;
	struct sockaddr_in server, client;
	pthread_t thread1;
	
	int c;
	strcpy(registerIds,"registered");

	//Create Socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("\nCould not create Gateway socket...");
	}
	
	puts("\nGateway socket is created");
	
	//Define SockAddr struct
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8080);	
	
	//Bind the socket and server address
	if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		perror("\nUnable to Bind socket and address");
		return 1;
	}	
	puts("Socket and address binded");
	puts("Waiting for registrations. . .\n");

	//Listen for connections
	listen(sockfd, 10);
	
	//-------------------------------------------------------------------------------------------------------	
	//connect_backend	
	struct sockaddr_in backend;
	//create the socket
	if((backsockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("\nCould not create Sensor socket");
	}

	//Initialise the backend socket
	backend.sin_family = AF_INET;
	backend.sin_addr.s_addr = inet_addr("127.0.0.1");
	backend.sin_port = htons( 8081 );

	//Connect to gateway
	if((connect(backsockfd, (struct sockaddr *) &backend, sizeof(backend))) < 0)
	{
		perror("\nUnable to connect to Gateway");
		return;
	}
	
	puts("gateway: Connected to backend");
	//-------------------------------------------------------------------------------------------------------
	
	c = sizeof(struct sockaddr_in);
	while (clientsockfd = accept(sockfd,(struct sockaddr *) &client, (socklen_t*)&c))
	{
		clientStruct1 cs1;
		cs1.sock = clientsockfd;
		//Accept a connection
		printf("\nAccepted a connection");
		
		if(pthread_create(&thread1, NULL, connection_handler, (void *) &cs1) < 0)
		{
			perror("\nJob thread creation failed");
			return 1;
		}
		
		puts("\nJob is taken");
	}
	
	if(clientsockfd < 0)
	{
		perror("\nGateway connection terminated");
		return 1;
	}
} 

void * connection_handler(void * cs1)
{	
	sleep(1);
	id++;
	int itemId = id;
    pthread_t thread1;
	clientStruct1 cs = *(clientStruct1*)(cs1);
	csArr[itemId].sock = cs.sock;	

	printf("csArr[%d].sock = %d\n",itemId,cs.sock);		
		
	int msglen;
	
	char readmsg[2000];
	char *regInfo, temp;
	char *writemsg;
	char tempo[200];
	
	writemsg = "Gateway: Connection established";
	
	//Send msg to Client
	write(csArr[itemId].sock, writemsg, strlen(writemsg)); 
	
	msglen = recv(csArr[itemId].sock, readmsg, 2000, 0);
	regInfo = (char *)readmsg;
	
	puts(regInfo);
	//Get Client Registration Information

	strcpy(csArr[itemId].name ,strtok(regInfo, ":"));
	strcpy(csArr[itemId].ip ,strtok(NULL,":"));
	csArr[itemId].port = atoi(strtok(NULL,":"));
	csArr[itemId].registered = 1;
	csArr[itemId].value = 0;
	csArr[itemId].idNum = itemId;
	
	strcpy(tempo, csArr[itemId].name);
	writemsg = (char *)tempo; 
	strcat(writemsg, " ");
	strcat(writemsg, csArr[itemId].ip);
	strcat(writemsg, " Registration complete.\nWaiting for other devices to connect before we can start\n"); 
	
	sleep(1);
	
	puts(writemsg);
	write(csArr[itemId].sock, writemsg, strlen(writemsg));
	memset(readmsg, 0, 2000);
	
	if((strstr(csArr[itemId].name, "door") != NULL) || (strstr(csArr[itemId].name, "motion") != NULL) || (strstr(csArr[itemId].name, "keychain") != NULL) || (strstr(csArr[itemId].name, "security") != NULL))
	{
		reg++;
	}
	//if received from all sensors, order all sensors to proceed
	//some code and logic here
	while(1)
	{
		if(id >= 3 && reg >= 3)
			break;
	}	
	
	// Generate only one common register message and pass to all sensors
	int i, port;
	char portStr[10];	
	if(setReg == 1)
	{
		setReg = 0;
		for(i=0;i<4;i++)
		{
			if(strstr(csArr[i].name, "security") == NULL) 
			{
				port = csArr[i].port;
				sprintf(portStr, "%d", port);
			
				strcat(registerIds, ":");
				strcat(registerIds, csArr[i].name);
				strcat(registerIds, ":");
				strcat(registerIds, csArr[i].ip);
				strcat(registerIds, ":");
				strcat(registerIds, portStr);
			}	
		}
	printf("\nmessage to pass to sensors = '''\n%s\n'''\n",registerIds);
	}		
	
	// Acknowledge sensors that everyone has been registered and they should now proceed.
	write(csArr[itemId].sock, registerIds, strlen(registerIds));
	pthread_create(&thread1, NULL, &threadReadFun, (void *)&csArr[itemId]);
																																																																																									
}

void* threadReadFun(void *cs1)
{
	int msglen, i, vectVal[4] = {0};
	char readmsg[2000];
	char sendmsg[2000];
	char vectInfo[50];
	char fileMsg[100];
	
	clientStruct1 cs = *(clientStruct1*)(cs1);
	while(msglen = recv(cs.sock, readmsg, 2000, 0) > 0 )
	{
		printf("Message received from %s = %s\n", cs.name, readmsg);
		sprintf(fileMsg, "Message received from %s = %s\n", cs.name, readmsg);
		
		pthread_mutex_lock(&mutex);
		writeToFile(&fileMsg);		
			
		strcpy(vectInfo, strtok(readmsg, ";"));//timestamp
		strcpy(vectInfo, strtok(NULL, ";"));//state
		
		for(i=0; i<3; i++)
		{
			vectVal[i] = atoi(strtok(NULL,";"));
			printf("vectVal[%d] = %d ; ",i,vectVal[i]);
			
			if(vectVal[i] > vectorClock[i])
			{
				vectorClock[i] = vectVal[i];
			}
		}	
		printf("\n");
		
		sprintf(sendmsg, "%d;%s;%s;%d;%s", cs.idNum, cs.name, cs.ip, cs.port, readmsg);
		write(backsockfd, sendmsg, strlen(sendmsg));				
		pthread_mutex_unlock(&mutex);		
		printf("Message sent to backend = %s\n",sendmsg);
		
		memset(sendmsg, 0, sizeof(sendmsg));
		memset(readmsg, 0, sizeof(readmsg));
	}
	if(msglen == 0)
	{
		printf("\nJob %s dropped\n", cs.name);
		fflush(stdout);
	}
	
	else if(msglen < 0)
	{
		perror("\nDisconnected "); 
	}
}

void writeToFile(char * fileData)
{
	fpOutputFile = fopen(file3, "a");
	fprintf(fpOutputFile, "%s", fileData);
	fclose(fpOutputFile);
}
