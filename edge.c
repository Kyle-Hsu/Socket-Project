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
#include <sys/types.h> // for socket()
#include <netinet/in.h> // for structs
#include <sys/socket.h> //for socket()
#include <arpa/inet.h>
#include <sys/wait.h>

#define HOST "localhost"
#define TCPPORT "23986"
#define UDPPORT "24986"
#define MAXDATASIZE 25
#define PORTOR "21986"
#define PORTAND "22986"

#define BACKLOG 10 // how many connections can be in queue

//The two functions copied from BEEJ 
void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(){
	//SET UP TCP SOCKET client side
	//code used from BEEJ example page 29
	int TCPsockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo TCPhints, *TCPservinfo, *TCPp; 
	struct sockaddr_storage TCPtheir_addr; // connector's address information
	socklen_t TCPsin_size;
	struct sigaction TCPsa;
	int yes=1;

	int TCPrv;
	memset(&TCPhints, 0, sizeof TCPhints);
	TCPhints.ai_family = AF_UNSPEC;
	TCPhints.ai_socktype = SOCK_STREAM;
	TCPhints.ai_flags = AI_PASSIVE; // use my IP

	//change the PORT to my TCPPORT number 
	if ((TCPrv = getaddrinfo(HOST, TCPPORT, &TCPhints, &TCPservinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(TCPrv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	// getaddrinfo put a linked list of addresses in servinfo

	for(TCPp = TCPservinfo; TCPp != NULL; TCPp = TCPp->ai_next) {


		//try to create the socket, if it fails where socket descriptor is -1
		//print error and try the next p
		TCPsockfd = socket(TCPp->ai_family, TCPp->ai_socktype, TCPp->ai_protocol);
		if (TCPsockfd == -1) {
			perror("server: socket");
			continue; 
		}

		//set socket options
		//code to reuse a port
		//avoid the "already in use" message
		if (setsockopt(TCPsockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1); 
		}


		//bind the socket to the port
		if (bind(TCPsockfd, TCPp->ai_addr, TCPp->ai_addrlen) == -1) {
			close(TCPsockfd);
			perror("server: bind");
			continue; 
		}
		break; 
	}

	freeaddrinfo(TCPservinfo); // all done with this structure

	if (TCPp == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	//listen on the port sockfd is bind to
	if (listen(TCPsockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf( "The edge server is up and running. \n");


	while(1) {  // main accept() loop

		// BEEJ create child socket new_fd
		// sin_size of connector's address
		TCPsin_size = sizeof TCPtheir_addr;
		new_fd = accept(TCPsockfd, (struct sockaddr *)&TCPtheir_addr, &TCPsin_size);
		if (new_fd == -1) {
			perror("accept");
			continue; 
		}


		int numLines;
		recv(new_fd, (char*)&numLines, sizeof (numLines), 0);
		
		char input[numLines][MAXDATASIZE];
		for(int i = 0; i < numLines; i++){
			recv(new_fd, &input[i], sizeof input[i], 0);
		}	
		
		printf("The edge server has received %d lines from the client using TCP over port %s.\n",numLines, TCPPORT);



		//PARSING THE LINE
		//remove the \n character from the strings
		for(int i = 0; i < numLines; i++){
			char* point = strtok(input[i], "\n");
			strcpy(input[i], point);
		}

		//maximum number of chars for operation or, and = 4;
		char operations[numLines][4];
		char binString1[numLines][10];
		char binString2[numLines][10];
		
		//Divide the line into three columns, into respective arrays
		char* point = NULL;
		for(int i = 0; i < numLines; i++){
			point = strtok(input[i],",");
			strcpy(operations[i], point);
			point = strtok(NULL,",");
			strcpy(binString1[i], point);		
			point = strtok(NULL,",");
			strcpy(binString2[i], point);
			point = NULL;

		}

		//Count the number of ors and ands
		int numOR = 0;
		int numAND= 0;
		for(int i = 0; i < numLines; i++){
			if(strcmp(operations[i], "and") == 0){
				numAND++;
			} else {
				numOR++;
			}
		}

		char paddedBstring1[numLines][11];
		char paddedBstring2[numLines][11];
		for(int i = 0; i < numLines; i++){
			
			int a1diff = 10 - strlen(binString1[i]);
			int a2diff = 10 - strlen(binString2[i]);

			if(a1diff >= 1){
				strcpy(paddedBstring1[i], "0");
			}

			for(int j = 1; j < a1diff; j++){
				strcat(paddedBstring1[i],"0");

			}

			int a1index = 0;
			for(int j = a1diff; j < 10; j++){
				if(binString1[i][a1index] == '0')
				{
					strcat(paddedBstring1[i], "0");
				}else{
					strcat(paddedBstring1[i], "1");
				}
				a1index++;
			}
			
			if(a2diff >= 1){
				strcpy(paddedBstring2[i], "0");
			}

			for(int j = 1; j < a2diff; j++){
				strcat(paddedBstring2[i],"0");

			}
			int a2index = 0;
			for(int j = a2diff; j < 10; j++){
				if(binString2[i][a2index] == '0')
				{
					strcat(paddedBstring2[i], "0");
				}else{
					strcat(paddedBstring2[i], "1");
				}
				a2index++;
			}
		}
		for(int i = 0; i < numLines; i++){
			printf("%s ", paddedBstring1[i]);
			printf("%s\n",paddedBstring2[i]);
		}



		//CREATE UDP PORT for AND REFERECING BEEJ
		int ANDsockfd;
		struct addrinfo ANDhints, *ANDservinfo, *ANDp;
		int ANDrv;
		struct sockaddr_storage ANDtheir_addr;
		socklen_t ANDaddr_len;

		memset(&ANDhints, 0, sizeof ANDhints);
		ANDhints.ai_family = AF_UNSPEC;
		ANDhints.ai_socktype = SOCK_DGRAM;
		ANDhints.ai_flags = AI_PASSIVE;

		if ((ANDrv = getaddrinfo(HOST, PORTAND, &ANDhints, &ANDservinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ANDrv));
			return 1;
		}
		// SAME AS TCP BIND TO FIRST AVAILABLE SOCKET()
		// loop through all the results and bind to the first we can
		for(ANDp = ANDservinfo; ANDp != NULL; ANDp = ANDp->ai_next) {
			if ((ANDsockfd = socket(ANDp->ai_family, ANDp->ai_socktype,
					ANDp->ai_protocol)) == -1) {
				perror("edge: socket");
				continue; 
			}
			if (setsockopt(ANDsockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					sizeof(int)) == -1) {
				perror("setsockopt");
				exit(1); 
			}
			break; 
		}

		if (ANDp == NULL) {
			fprintf(stderr, "edge: failed to bind socket\n");
			return 2;
		}
		freeaddrinfo(ANDservinfo);



		sendto(ANDsockfd, (char *)&numAND, sizeof numAND, 0, ANDp->ai_addr, ANDp->ai_addrlen);
		for(int i = 0; i < numLines; i++){
			if(strcmp(operations[i], "or") != 0){
				sendto(ANDsockfd, (char*)&i, sizeof i, 0, ANDp->ai_addr, ANDp->ai_addrlen);
				sendto(ANDsockfd, paddedBstring1[i], sizeof paddedBstring1[i], 0, ANDp->ai_addr, ANDp->ai_addrlen);
				sendto(ANDsockfd, paddedBstring2[i], sizeof paddedBstring2[i], 0, ANDp->ai_addr, ANDp->ai_addrlen);	
			}
		}
		printf("The edge has successfully sent %d lines to Backend-Server AND.\n", numAND);
		
		int ANDsequence[numAND];
		memset(ANDsequence, 0, numAND);
		int ANDresults[numAND];
		memset(ANDresults, 0, numAND);

		for(int i = 0; i < numAND; i++){
			recvfrom(ANDsockfd, (char*)&ANDsequence[i], sizeof ANDsequence[i], 0, NULL, NULL);
			recvfrom(ANDsockfd, (char*)&ANDresults[i], sizeof ANDresults[i], 0, NULL, NULL);
		}

		for(int i = 0; i < numAND; i++){
			printf("and results: %d %d\n",ANDsequence[i], ANDresults[i]);
		}


		close(ANDsockfd);


		//CREATE UDP SOCKET FOR OR REFERENCING BEEJ
		int ORsockfd;
		struct addrinfo ORhints, *ORservinfo, *ORp;
		int ORrv;
		struct sockaddr_storage ORtheir_addr;
		socklen_t ORaddr_len;

		memset(&ORhints, 0, sizeof ORhints);
		ORhints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
		ORhints.ai_socktype = SOCK_DGRAM;
		ORhints.ai_flags = AI_PASSIVE; // use my IP

		if ((ORrv = getaddrinfo(HOST, PORTOR, &ORhints, &ORservinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ORrv));
			return 1;
		}
		// SAME AS TCP BIND TO FIRST AVAILABLE SOCKET()
		// loop through all the results OR bind to the first we can
		for(ORp = ORservinfo; ORp != NULL; ORp = ORp->ai_next) {
			if ((ORsockfd = socket(ORp->ai_family, ORp->ai_socktype,
					ORp->ai_protocol)) == -1) {
				perror("edge: socket");
				continue; 
			}
			if (setsockopt(ORsockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
					sizeof(int)) == -1) {
				perror("setsockopt");
				exit(1); 
			}
			break; 
		}

		if (ORp == NULL) {
			fprintf(stderr, "edge: failed to bind socket\n");
			return 2;
		}
		freeaddrinfo(ORservinfo);


		sendto(ORsockfd, (char *)&numOR, sizeof numOR, 0, ORp->ai_addr, ORp->ai_addrlen);
		for(int i = 0; i < numLines; i++){
			if(strcmp(operations[i], "and") != 0){
				sendto(ORsockfd, (char*)&i, MAXDATASIZE-1, 0, ORp->ai_addr, ORp->ai_addrlen);
				sendto(ORsockfd, paddedBstring1[i], sizeof paddedBstring1[i], 0, ORp->ai_addr, ORp->ai_addrlen);
				sendto(ORsockfd, paddedBstring2[i], sizeof paddedBstring2[i], 0, ORp->ai_addr, ORp->ai_addrlen);
			}
		}

		int ORsequence[numOR];
		memset(ORsequence, 0, numOR);
		int ORresults[numOR];
		memset(ORresults, 0, numOR);

		for(int i = 0; i < numOR; i++){
			recvfrom(ORsockfd, (char*)&ORsequence[i], sizeof ORsequence[i], 0, NULL, NULL);
			recvfrom(ORsockfd, (char*)&ORresults[i], sizeof ORresults[i], 0, NULL, NULL);
		}

		for(int i = 0; i < numOR; i++){
			printf("or results: %d %d\n",ORsequence[i], ORresults[i]);
		}



		printf("The edge has successfully sent %d lines to Backend-Server OR.\n", numOR);



		close(ORsockfd);

		printf("The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP over port %s.\n", UDPPORT);

		int results[numLines];
		for(int i = 0; i < numAND; i++){
			results[ANDsequence[i]] = ANDresults[i];
		}

		for(int i = 0; i < numOR; i++){
			results[ORsequence[i]] = ORresults[i];
		}
		
		for(int i = 0; i < numLines; i++){
			printf("%s %s %s = %d\n",binString1[i],operations[i], binString2[i], results[i]);
		}

		printf("The edge server has successfully finished receiving all computation results from Backend-Server OR and Backend-Server AND.\n");

		for(int i = 0; i < numLines; i++){
			send(new_fd, (char*)&results[i], sizeof results[i], 0);
		}


		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
