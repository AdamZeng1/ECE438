/*
 * File:   receiver_main.c
 * Author:
 *
 * Created on
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <map>

using namespace std;

struct sockaddr_in si_me, si_other;
int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}


//This function is reliablyTransfer’s counterpart, and should write what it receives to a file called destinationFile.
void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {

    slen = sizeof (si_other);


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");


	/*TODO: Now receive data and send acknowledgements */
    //Data structure and constant initialization
    map<int, string> receivedBufferedMap;    //c++??
    long long int lastAckedNum = -1;
    long long int orgPlanSeq = 0;

    char buf[1600];
    FILE* fp;

    struct sockaddr_in their_addr; //sender's address, get filled from recvfrom function(how does this work?)
    int numbytes = 0;
    socklen_t their_addr_size = sizeof(their_addr);
    //ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen);
    while (1) {
        if (numbytes = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&their_addr, &their_addr_size) < 0 ) {
            diep("Finished: receive ends!");
        }

        char seqBuf[32];
        long long int currSeqNum;
        char contentBuf[1600];
        char* bufPtr = buf;
        int seqI;
        int contentI;

        //1.get seq num
        for (seqI = 0; seqI < sizeof(buf); seqI++, bufPtr++) { //sizeof ?
            if (*(bufPtr) == ',') {
                //seqBuf[seqI + 1] = '/0';
                break;
            }
            seqBuf[seqI] = *bufPtr;
        }
        sscanf(seqBuf, "%lld", &currSeqNum);   //string to int

        //2.get content
        bufPtr++;
        for (contentI = seqI + 1; contentI < sizeof(buf); contentI++, bufPtr++) {
            contentBuf[contentI - seqI - 1] = *bufPtr;
        }
        //contentBuf[contentI - seqI - 1] = '/0'; //right??
        string contentStr = contentBuf; //c++

        //a packet that arrives too early, buffer it
        if (currSeqNum > orgPlanSeq && (receivedBufferedMap.find(currSeqNum) == receivedBufferedMap.end() || receivedBufferedMap[currSeqNum] == "")) {
            receivedBufferedMap[currSeqNum] = contentStr;
        }
        //a packet which is expected, deliver it, and go forward to deliver any buffered packets right behind it as well
        else if (currSeqNum == orgPlanSeq) {
            // write to file and deliver
            fwrite(contentBuf, 1, strlen(contentBuf), fp);
            fflush(fp);
            //release the buffered packets and write them all
            long long int nextPktIdx = currSeqNum + 1;
            while (receivedBufferedMap.find(nextPktIdx) != receivedBufferedMap.end() && receivedBufferedMap[nextPktIdx].length() != 0) {
                fwrite(receivedBufferedMap[nextPktIdx].c_str(), 1, receivedBufferedMap[nextPktIdx].length(), fp);
                fflush(fp);
                // writeToFileAndDeliver(receivedBufferedMap[nextPktIdx]);
                receivedBufferedMap[nextPktIdx] = "";
                nextPktIdx ++;
            }
            lastAckedNum = nextPktIdx - 1;
            orgPlanSeq = nextPktIdx;
        }
    }


    close(s);
	printf("%s received.", destinationFile);
    return;
}

/*
 *
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}
