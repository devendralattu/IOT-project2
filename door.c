//Door

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

void * connection_handler(void *);
void * connection_handler_2(void *);
void * doorServerThread(void * args);
void * doorConnectMotionThread(void * args);
void * doorConnectKeychainThread(void * args);

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

struct sockaddr_in motion, keychain;
int sockfdMotion = 0, sockfdKeychain = 0;
int vectorClock[4] = {0};
	
int main(int argc, char* argv[])
{
	char* file1 = argv[1];
	char* file2 = argv[2];
	
	file1 = "DoorConfigurationFile.txt";
	file2 = "DoorStateFile.txt";

	int sockfd;
	struct sockaddr_in server;
	char readmsg[2000], msglen;
	pthread_t doSerThread, doConnThread;	
		
	//To read the Config File 
	FILE *fp1 = fopen(file1,"r");	
	
	//To Get registration data from the File
    int i, j;
    size_t len;
    char *gatewayIP; 
    char *gatewayPort; 
    char *gatewayAddress = (char *)malloc(sizeof(char)*100);
    char *configInfo = (char *)malloc(sizeof(char)*100);
    
	//1st line
    getline(&gatewayAddress, &len, fp1);
    gatewayIP = strtok(gatewayAddress, ":");
    gatewayPort = strtok(NULL, "\n");
    
    //2nd line
    getline(&configInfo, &len, fp1);
    fclose(fp1);
    
	//create the socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("\nCould not create Sensor socket");
	}
	
	//Initialise the server socket
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(gatewayIP);
	server.sin_port = htons( atoi(gatewayPort) );
	
	//Connect to gateway
	if(connect(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		perror("\nUnable to connect to Gateway");
		return 1;
	}
	
	puts("Sensor: Connected to Gateway");
	
	msglen = read(sockfd, readmsg, 2000);
	puts(readmsg);
	memset(readmsg, 0, 2000);
	
	write(sockfd, configInfo, strlen(configInfo), 0);
    puts(configInfo);
    
	msglen = recv(sockfd, readmsg, 2000, 0);
	readmsg[msglen] = '\0';
	puts(readmsg);
	
	//get message to proceed!
	msglen = recv(sockfd, readmsg, 200, 0);
	readmsg[msglen] = '\0';
	puts(readmsg);
	while(strcmp(readmsg, "registered") < 0)
	{
		puts("Gateway asked to proceed");
		break;	
	}
	
	sleep(1);	
	//Create server and connect with other sensors
	pthread_create(&doSerThread, NULL, &doorServerThread, NULL);	
	
	sleep(5);
	//Connect with other sensors
	pthread_create(&doConnThread, NULL, &doorConnectMotionThread, NULL);	
	pthread_create(&doConnThread, NULL, &doorConnectKeychainThread, NULL);	
	
	//To read state file and send values to Gateway
	FILE *fp2 = fopen(file2,"r");
	char *state = (char *)malloc(sizeof(char)*100);
	char copyState[20];
	int nextTime;
	char *value;
	char *temp;
	char msgToOtherSensors[2000];
		
	i = 0;
	getline(&state, &len, fp2);
	
	temp = strtok(state,";");
	nextTime = atoi(temp);
	value = strtok(NULL,";");
	sprintf(copyState, "%d;%s", time(NULL), value);
	//Pushing values to Gateway
	while(i <= nextTime)
	{
		if(i == nextTime)
		{
			write(sockfd, copyState, strlen(copyState));
			
			vectorClock[2] = vectorClock[2] + 1;
			printf("[%d,%d,%d]",vectorClock[0],vectorClock[1],vectorClock[2]);
				
			sprintf(msgToOtherSensors, "door-%s", copyState);
			if(sockfdMotion > 0 && sockfdKeychain > 0)
			{
				write(sockfdMotion, msgToOtherSensors, strlen(msgToOtherSensors));
				write(sockfdKeychain, msgToOtherSensors, strlen(msgToOtherSensors));
			}
			
			puts(copyState);
			if(getline(&state, &len, fp2) < 0)
				break;
				
			temp = strtok(state,";");
			nextTime = atoi(temp);
			value = strtok(NULL,";");
		}
		sleep(1);
		sprintf(copyState, "%d;%s",time(NULL), value);
		i++;
	}
	fclose(fp2);
	return 0;
} 

void * doorServerThread(void * args)
{	
	int sockfdDoor, clientsockfd;
	struct sockaddr_in serverDoor, client;
	pthread_t thread1;
	int c;
	
	//Create Socket
	if((sockfdDoor = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		printf("\nCould not create Door socket...");
	}
	
	puts("\nDoor socket is created");
	
	//Define SockAddr struct
	serverDoor.sin_family = AF_INET;
	serverDoor.sin_addr.s_addr = INADDR_ANY;
	serverDoor.sin_port = htons(8084);	
	
	//Bind the socket and server address
	if(bind(sockfdDoor, (struct sockaddr *) &serverDoor, sizeof(serverDoor)) < 0)
	{
		perror("\nDoor : Unable to Bind socket and address");
	}	
	puts("Door : Socket and address binded");
	puts("Door : Waiting for registrations. . .\n");

	//Listen for connections
	listen(sockfdDoor, 10);
	
	printf("sockfdDoor = %d\n",sockfdDoor);
	sleep(1);
	
	c = sizeof(struct sockaddr_in);
	//Accept a connection
	while (clientsockfd = accept(sockfdDoor,(struct sockaddr *) &client, (socklen_t*)&c))
	{
		clientStruct1 cs1;
		cs1.sock = clientsockfd;
		
		printf("\nAccepted a connection");
		
		if(pthread_create(&thread1, NULL, connection_handler, (void *) &cs1) < 0)
		{
			perror("\nDoor : Job thread creation failed");
		}
		
		puts("\nDoor : Job is taken");
	}
	
	if(clientsockfd < 0)
	{
		perror("\nDoor connection terminated");
	}
}

void * connection_handler(void * cs1)
{		
	pthread_t doConn2Thread;
	clientStruct1 cs = *(clientStruct1*)(cs1);

	char *writemsg = "Message from door";
	int msglen;
	char readmsg[2000];
	
	printf("Writing message from door to cs.sock = %d\n",cs.sock);			
	
	// ------ receive message ---------
	pthread_create(&doConn2Thread, NULL, &connection_handler_2, (void *) &cs);
}

void * connection_handler_2(void * cs2)
{
	clientStruct1 cs_r = *(clientStruct1*)(cs2);
	char *writemsg = "Message from door";
	int msglen;
	char readmsg[2000];
	
	printf("Reading message from cs_r.sock = %d\n",cs_r.sock);			
	while(msglen = recv(cs_r.sock, readmsg, 2000, 0) > 0)
	{
		puts(readmsg);
		if(strstr(readmsg, "motion-") != NULL)
		{
			vectorClock[1] = vectorClock[1] + 1;
		}
		if(strstr(readmsg, "keychain-") != NULL)
		{
			vectorClock[0] = vectorClock[0] + 1;
		}
		printf("[%d,%d,%d]\n",vectorClock[0],vectorClock[1],vectorClock[2]);
		memset(readmsg, 0, 2000);
	}
	if(msglen == 0)
	{
		printf("\nJob dropped\n");
		fflush(stdout);
	}
}

void * doorConnectMotionThread(void * args)
{
	//create MOTION socket
	if((sockfdMotion = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("\nCould not create sockfdMotion Sensor socket");
	}

	//Initialise the backend socket
	motion.sin_family = AF_INET;
	motion.sin_addr.s_addr = inet_addr("127.0.0.1");
	motion.sin_port = htons(8082);

	//Connect to gateway
	if((connect(sockfdMotion, (struct sockaddr *) &motion, sizeof(motion))) < 0)
	{
		perror("\nUnable to connect to motion");
	}
	
	puts("door: Connected to motion");	
}

void * doorConnectKeychainThread(void * args)
{	
	//create KEYCHAIN socket
	if((sockfdKeychain = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("\nCould not create sockfdKeychain Sensor socket");
	}

	//Initialise the backend socket
	keychain.sin_family = AF_INET;
	keychain.sin_addr.s_addr = inet_addr("127.0.0.1");
	keychain.sin_port = htons(8083);

	//Connect to gateway
	if((connect(sockfdKeychain, (struct sockaddr *) &keychain, sizeof(keychain))) < 0)
	{
		perror("\nUnable to connect to keychain");
	}
	
	puts("door: Connected to keychain");
}
