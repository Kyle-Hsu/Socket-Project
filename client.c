/*
Name: Kai-Yun Hsu
Session 2
Spring 2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define EDGEPORT "23986" // the edge server port client will be connecting to
#define MAXROWS 100
#define MAXDATASIZE 25
#define HOST "localhost"

// code from BEEJ page 31 TCP Stream Client
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd;

	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];


	if (argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;


	if ((rv = getaddrinfo(HOST, EDGEPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue; 
		}


		//tcp connection to an available socket
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue; 
		}
		break; 
	}
	
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	


	// inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	// 		s, sizeof s);
	

	printf("The client is up and running.\n");

	freeaddrinfo(servinfo); // all done with this structure
	
	char input[100][25];
	FILE *fp = fopen(argv[1], "r");
	int numLines = 0;
	while(fgets(input[numLines], MAXDATASIZE, fp)){
		numLines++;
	}
	fclose(fp);

	send(sockfd, (char*)&numLines, sizeof (numLines), 0);
	for(int i = 0; i < numLines; i++){
		send(sockfd, &input[i], sizeof (input[i]), 0);
	}



	printf("The client has successfully finished sending %d lines to the edge server.\n",numLines);


	int results[numLines];
	for(int i = 0; i < numLines; i++){
		recv(sockfd, (char*)&results[i], sizeof results[i], 0);
	}	


	printf("The client has successfully finished receiving all computation results from the edge server.\n");


	printf("The final computation results are:\n");

	for(int i = 0; i < numLines; i++){
		printf("%d\n",results[i]);
	}


	close(sockfd);

	return 0; 
}