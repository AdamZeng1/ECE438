/*
** client.c -- a stream socket client demo
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

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100000 // max number of bytes we can get at once

//Note that the newlines are technically supposed to be CRLF – so, “\r\n” on a Unix machine.
#define HEAD "GET /%s HTTP/1.1\r\n\
User-Agent: Wget/1.12(linux-gnu)\r\n\
Host: %s:%s\r\n\
Connection: Keep-Alive\r\n\r\n" //HTTP specifies that the end of a request should be marked by a blank line — so be sure to have two newlines at the end


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void get_input(char * input, char * filename, char * ipaddr, char * port){
	char * p_start;
	char * p_end;
	p_start = input;
	while(*p_start != '/'){
		p_start++;
	}
	p_start += 2;
	p_end = p_start;
	int flag = 0;
	int port_flag = 0;
	while(*p_end != '\x00'){
		if(*p_end == ':'){
			port_flag = 1;

			strncpy(ipaddr, p_start, p_end - p_start);
			p_end ++;
			p_start = p_end;
			continue;
		}
		if(flag == 0  && *p_end == '/'){

			if(port_flag == 1){
				strncpy(port, p_start, p_end - p_start);
			}
			else{
				strncpy(ipaddr, p_start, p_end - p_start);
			}
			p_end ++;
			p_start = p_end;
			flag = 1;

		}
		p_end++;
	}
	strncpy(filename, p_start, p_end - p_start);
//	return;
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;

	// heap
	char buf[MAXDATASIZE];

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


	//char filename[128];
	char filename[1024];
	char port[8];
	//char * path = NULL;
	//char ipaddr[16];
	char ipaddr[1024];
	memset(filename, 0, sizeof filename);
	memset(port, 0, sizeof port);
	memset(ipaddr, 0, sizeof ipaddr);

	get_input(argv[1], filename, ipaddr, port);

	char * Port = PORT;
	if(*port != '\x00') {
		 Port = port;
	}

//	printf("%s, %s, %s", fid, ipaddr, Port);
//	exit(0);

	if ((rv = getaddrinfo(ipaddr, Port, &hints, &servinfo)) != 0) {
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

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);
	//printf("client: connecting to %s\n", s);
	//printf("test");


// **************************
	//char * content;

	/*
	FILE * http_req = fopen(filename, "rb");
	if(http_req == NULL){
		printf("failure occurred when opening");
	}
	fseek(http_req, 0, SEEK_END);
	int file_len = ftell(http_req);
	rewind(http_req);
	*/
	//printf("client: connecting to %s\n", s);
	//printf("ready to set send buff");
	char send_buff[1024];
	memset(send_buff, 0, sizeof send_buff);
	sprintf(send_buff, HEAD, filename, ipaddr, Port);
	int total_len = strlen(send_buff);
	//printf("%d\n", total_len);
	/*fread(content, file_len, 1, http_req);
	fclose(http_req);
	*/
	int sent = 0;
	while(sent < total_len){
		//printf("%d\n", total_len);
		//sleep(2);
		printf("sent: %d\n", sent);
		int n = send(sockfd, send_buff, total_len - sent, 0);
		printf("n: %d\n", n);
		if(n == -1){
			break;
			printf("failure when sending file");
		}
		sent = sent + n;
	}

//printf("%d\n", total_len);

	freeaddrinfo(servinfo); // all done with this structure


/*	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}
	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

*/
	int received = 0;
	int n = 1;
	int headerRemoved = 0;
	char * headerEndPtr;
	FILE * fp = fopen(filename, "wb");

/*
	while (1) {
		n = recv(sockfd, buf, MAXDATASIZE-1, 0);
		if (n == 0) {
			break;
		}
	}*/

	//headerEndPtr = strstr(buf, "\r\n\r\n") + 4;
	//fwrite(headerEndPtr, sizeof (char), strlen(buf) - (headerEndPtr - buf), fp);


	// endptr
	do {
		n = recv(sockfd, buf, MAXDATASIZE-1, 0);
		printf("n:%d\n",n);

		//********* socket hang for while

		if (n == 0){
			break;
		}

		if (n == -1) {
			perror("recv");
			exit(1);
		}

		//
		headerEndPtr = strstr(buf, "\r\n\r\n") + 4;
		if (!headerRemoved) {
			fwrite(headerEndPtr, sizeof (char), n + buf - headerEndPtr, fp);
			headerRemoved = 1;
			//************ modified*************
			n = n + buf - headerEndPtr;
		} else {
			fwrite(buf, sizeof (char), n, fp);
		}

		received += n;

	} while(1);

	fclose(fp);
	close(sockfd);
	printf("finished receiving file, the size is: %d Bytes \n", received);
	return 0;
}
