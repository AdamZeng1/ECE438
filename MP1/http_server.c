/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 100000

//#define TAIL "\n\n"
/*#define HEAD "HTTP/1.1 200 OK\n\
Content-Type:text/html,charset=ascii\n\
Connection:Keep-Alive\n\
Cache-Control:private,max_age=0\n\
Transfer-Coding:chunked\n\
Accepted-Ranges:bytes\n\
Content-Length:%d\n\n"*/

//#define TAIL ""
#define HEAD "HTTP/1.1 200 OK\r\n\r\n"

char LOGBUFF[1024];
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void save_log(char * buff){
	FILE * fp = fopen("log.txt", "a+");
	fputs(buff, fp);
	fclose(fp);
}


// delimit the http text and get the filename, HTTP version
void preprocess_http_request(char * httpmsg, char * command){
	char * start_req = httpmsg;
	char * end_req = httpmsg;

	while(*start_req){
		if (*start_req == '/'){
			break;
		}
		start_req++;
	}

	start_req++;
	end_req = strchr(httpmsg, '\n');

	while(*end_req != ' '){
		end_req--;
	}
	strncpy(command, start_req, end_req - start_req);

}

int get_content(const char * filename, char ** content){


	FILE * fp = NULL;
	int file_len = 0;

	if (filename == NULL){
		 return file_len;
	}

	fp = fopen(filename, "rb");
	if(fp == NULL){
		// error info
		memset(LOGBUFF, 0, sizeof LOGBUFF);
		sprintf(LOGBUFF, "file open failure: %s\n", strerror(errno) );
		save_log(LOGBUFF);
		exit(EXIT_FAILURE);
	}
	fseek(fp, 0, SEEK_END);
	file_len = ftell(fp);
	rewind(fp);

	//content saved on heap
	*content = (char *)malloc(file_len);
	// memset ?? which will be better ******************
	fread(*content, file_len, 1, fp);
	fclose(fp);

	return file_len;
}

// command only should be the string after the method 'GET'
int generate_http_response(const char * command, char ** content){

	char * file_buff = NULL;
	char head_buff[1024];
	int file_length;



	// file_buf is the addr of the string
	file_length = get_content(command, &file_buff);

//	printf("file_length: %d", file_length);

	if (file_length == -1 || file_length == 0){
		//error
	}

	memset(head_buff, 0, sizeof head_buff);
	sprintf(head_buff, HEAD);
	int head_len = strlen(head_buff);
	//int tail_len = sizeof(TAIL);

	int total_len = head_len + file_length;
	//int total_len = head_len + tail_len + file_length;
	*content = (char* ) malloc(total_len);

	char * tmp = *content;
	memcpy(tmp, head_buff, head_len);
	memcpy(&tmp[head_len], file_buff, file_length);
	//memcpy(&tmp[head_len] + file_length, TAIL, tail_len);

	if (file_buff){
		free(file_buff);
	}

	return total_len;
}



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[])
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

	if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		printf("check%d\n",1);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		printf("check%d\n",2);
		printf("new_fd:%d\n", new_fd);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		//read file content and parse it to request preprocess
		//recv(sockfd, buff, len, flag)
		char buffer[1024];
		char command[1024];
		memset(buffer, 0, sizeof buffer);
		memset(command, 0, sizeof command);
		printf("check%d\n",3);
		rv = recv(new_fd, buffer,sizeof buffer, 0);
		printf("rv:%d",rv);
		if(rv == 0){
			memset(LOGBUFF, 0, sizeof LOGBUFF);
			sprintf(LOGBUFF, "receive no messages along the connection");
		}

		// s == char[ipv6_length]
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		// multi-process
		if (!fork()) { // this is the child process
			// there is no need to shutdown the listener, but should create a new thread for the new sockfd
			//
			close(sockfd); // child doesn't need the listener


			preprocess_http_request(buffer, command);

			//char * content = NULL;
			char content[MAXDATASIZE];
			FILE* fp;
			int sent = 0;
			int n;
			int firstRead = 1;

			//int response_len = generate_http_response(command, content);
			fp = fopen(command, "rb");
			memset(content, '\0', sizeof content);
			sprintf(content, HEAD);

			while(1){
				if (firstRead) {
					n = fread(content + strlen(HEAD), sizeof (char), MAXDATASIZE - strlen(HEAD), fp);
					n = send(new_fd, content, n + strlen(HEAD), 0);
					if (n == -1){
						printf("error occurred and the packets have not been sent completely");
						break;
					}
					firstRead = 0;
				} else {
					n = fread(content, sizeof (char), MAXDATASIZE, fp);
					n = send(new_fd, content, n, 0);
					if (n == -1){
						printf("error occurred and the packets have not been sent completely");
						break;
					}
				}
				//printf("sent:%d\n",sent);
				if (n == 0) {
					break;
				}

				sent += n;
			}

			// while(sent < response_len){
			// 	printf("sent:%d\n",sent);
			// 	int n = send(new_fd, tmp, response_len - sent, 0);
			// 	if (n == -1){
			// 		printf("error occurred and the packets have not been sent completely");
			// 		break;
			// 	}
			// 	sent += n;
			// }

			/*if ( send(new_fd, content, response_len, 0) == -1){

				memset(LOGBUFF, 0, sizeof LOGBUFF);
				sprintf(LOGBUFF, "send error: %s\n", strerror(errno));
				save_log(LOGBUFF);
				exit(EXIT_FAILURE);
			}*/
			//printf("success sending out %d bytes\n", response_len);

			close(new_fd);
			// if(content){
			// 	free(content);
			// }
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

/*int main(void){
	char * fn = "input";
	char * content = NULL;
	int file_length = get_content(fn, &content);
	FILE * fp = fopen("output", "wb");
	fwrite(content, file_length,1, fp);
	fclose(fp);

	//printf("write finished, and file_length equals %d\n",file_length);

	char command[1024];
	memset(command, 0, 1024);
	char http[] = "GET /input HTTP/1.1\n";
	preprocess_http_request(http, command);
//	printf("preprocess finished: %s\n", command);

	// test response
	int response_len = generate_http_response(command,&content);
//	printf("response_finished\noutput string: %s", content);

	FILE * fp = fopen("output", "wb");
	fwrite(content, response_len,1, fp);
	fclose(fp);

//	printf("response generate finished, and file_length equals %d\n",response_len);

	return 0;
}*/
