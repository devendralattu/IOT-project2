//MotionSensor

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	char* file1 = argv[1];
	char* file2 = argv[2];
	
	file1 = "MotionSensorConfigurationFile.txt";
	file2 = "MotionSensorStateFile.txt";

	int sockfd;
	struct sockaddr_in server;
	char readmsg[2000], msglen;
			
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
	
	write(sockfd, configInfo, strlen(configInfo));
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
	
	//To read state file and send values to Gateway
	FILE *fp2 = fopen(file2,"r");
	char *state = (char *)malloc(sizeof(char)*100);
	char *temp;
	int currTime;
	int nextTime = 0;
	char *value;
	char valCopy[5];
			
	//Getting individual parts of state file : initial time, end time, value
	i = 0;
	
	//Sending values to gateway every 5 seconds
	while(i <= nextTime)
	{
		if(i == nextTime)
		{		
			if(getline(&state, &len, fp2) < 0)
			{
				sprintf(temp, "%d;%s",time(NULL), valCopy); //Changed
				write(sockfd, temp, strlen(temp));
				puts(temp);
				break;
			}
			temp = strtok(state, ";");
			currTime = atoi(temp);
			temp = strtok(NULL, ";");
			nextTime = atoi(temp);
			value = strtok(NULL, ";");
			strcpy(valCopy, value);
		}
		
		if((i % 5) == 0)
		{	
			sprintf(temp, "%d;%s", time(NULL), valCopy); //Changed
			write(sockfd, temp, strlen(temp));
			puts(temp);
		}
		
		sleep(1);
		i++;
	}
	fclose(fp2);
	return 0;
} 
