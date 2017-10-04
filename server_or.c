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
#include <sys/wait.h>

#define PORTOR "21986"   //Port number for OR
#define HOST "localhost"
#define MAXDATASIZE 25

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



//SET UP UDP DATAGRAM SERVER REFERENCING BEEJ
int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXDATASIZE];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	int yes = 1;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORTOR, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue; 
		}

		//ADDED SETSOCKOPT SO I CAN REUSE
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1); 
		}
	    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				perror("listener: bind");
			continue; 
		}
		break; 
	}


	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}


	freeaddrinfo(servinfo);

	printf( "The Server OR is up and running using UDP on port %s.\n", PORTOR);


	while(1){
		addr_len = sizeof their_addr;
		int numOR;
		if ((numbytes = recvfrom(sockfd, (char *)& numOR, MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		printf("%d\n", numOR);
		int sequence[numOR];
		char binString1[numOR][11];
		char binString2[numOR][11];
		for(int i = 0; i < numOR; i++){
			recvfrom(sockfd, (char*)&sequence[i], MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
			recvfrom(sockfd, binString1[i], MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
			recvfrom(sockfd, binString2[i], MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
			printf("SEQ: %d %s OR %s = \n",sequence[i], binString1[i],binString2[i] );
		}

		printf("The Server OR has successfully received %d lines from the edge server and finished all OR computations.\n", numOR);

		char results[numOR][11];


		for(int i = 0; i < numOR; i++)
		{
			for(int j = 0; j < 10; j++){
				results[i][j] = binString1[i][j] | binString2[i][j];
			}
			results[i][10] = '\0';
		}

		int numStr1[numOR];
		int numStr2[numOR];
		int numResults[numOR];

		for(int i = 0; i < numOR; i++){
			numStr1[i] = atoi(binString1[i]);
			numStr2[i] = atoi(binString2[i]);
			numResults[i] = atoi(results[i]);
		}


		for(int i = 0; i < numOR; i++){
			printf("%d or %d = %d\n", numStr1[i],numStr2[i],numResults[i]);
		}

		printf("The Server OR has successfully received %d lines from the edge server and finished all OR computations.\n", numOR);

		for(int i = 0; i < numOR; i++){
			sendto(sockfd, (char*)&sequence[i], MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, addr_len);
			sendto(sockfd, (char*)&numResults[i], MAXDATASIZE-1 , 0, (struct sockaddr *)&their_addr, addr_len);
		}		




		printf("The Server OR has successfully finished sending all computation results to the edge server.\n");


	}

	return 0; 
}