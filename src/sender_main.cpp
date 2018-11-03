/*
 * File:   sender_main.c
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
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>


// struct sockaddr_in {
//     short int sin_family; // Address family, AF_INET
//     unsigned short int sin_port; // Port number
//     struct in_addr sin_addr; // Internet address
//     unsigned char sin_zero[8]; // 与 struct sockaddr 相同的大小
// };
struct sockaddr_in si_other;
int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}

//This function should transfer the first bytes of filename to the receiver at correctly and efficiently, even if the network drops or reorders some of your packets.
void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    //Open the file
    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }

	/* Determine how many bytes to transfer */

    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);

    //inet_aton() 將句號與數字組成的字串格式轉換到 in_addr_t（你 struct in_addr 中 s_addr 欄位的型別）。
    //例如：将192.168.5.10转为in_addr_t格式
    // struct in_addr {
    //     uint32_t s_addr; // that's a 32-bit int (4 bytes)
    // };
    //inet_aton()返回值:若 address 是合法的，則傳回非零的值，而若位址是非法的，則傳回零。
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


	/* Send data and receive acknowledgements on s*/

    printf("Closing the socket\n");
    close(s);
    return;

}

/*
 *
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);


    //char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer
    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);


    return (EXIT_SUCCESS);
}
