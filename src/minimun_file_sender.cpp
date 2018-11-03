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

// some constant var
#define INIT_SST  20
#define INIT_CWND_START  0
#define INIT_CWND_SIZE  1
#define BLOCK_SIZE_FOR_HEADER  12

//Byte 1472 - 8 MTU = 1500 udp_header + ipheader = 28; for 1 GB = 10e9B = 10e6 pkt;
#define BLOCK_SIZE_FOR_DATA  1464
#define PACKET_BUFFER_SIZE  1000


// c++ library and Packet class
#include"test_obj.h"
#include<iostream>
using namespace std;


int s, slen; // slen: the length of sockaddr_in, s:socket
struct sockaddr_in si_other;


int cwnd_start, cwnd_end; // set to be INIT_CWND_START
//
int ss_threshold, cwnd_size; // set to be INIT_SST, 1
bool wait_flag; // set to be false when init
bool timeout_flag; // set to be false when init
bool transmission_finished_flag; // set to be false when int

Packet * packet_window;

int current_transmit_round; // set to be 1 when init, plus one if circlely using the packet_window
int total_round;
unsigned long long int packet_total_numbers; // total number of packets = total_bytes / 1464
int last_packet_size;
long int current_time;
time_t timer;

void diep(char *s) {
    perror(s);
    exit(1);
}

void printBits(size_t const size, void const * const ptr){
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    // for (i=size-1;i>=0;i--)
    for (i = 0; i < size; i++)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

void init(unsigned long long int numberBytes){
  cwnd_start = INIT_CWND_START;
  cwnd_size = INIT_CWND_SIZE;
  cwnd_end = cwnd_start + cwnd_size;
  ss_threshold = INIT_SST;

  wait_flag = false;
  timeout_flag = false;
  transmission_finished_flag = false;

  // packet_window = {};
  current_transmit_round = 1;

  last_packet_size = numberBytes % BLOCK_SIZE_FOR_DATA;
  if(last_packet_size == 0){
    packet_total_numbers = numberBytes / BLOCK_SIZE_FOR_DATA;
  }else{
    packet_total_numbers = numberBytes / BLOCK_SIZE_FOR_DATA + 1;
  }

  int rd = packet_total_numbers % PACKET_BUFFER_SIZE;
  if(rd == 0){
    total_round = packet_total_numbers / PACKET_BUFFER_SIZE;
  }else{
    total_round = packet_total_numbers / PACKET_BUFFER_SIZE + 1;
  }

  packet_window = new Packet[PACKET_BUFFER_SIZE];
  memset(packet_window, 0, 8*PACKET_BUFFER_SIZE);

}

/* prepare file data into Packets, append timestamp info */
Packet* prepareData(FILE * fp, unsigned long long int sequenceNumber){
  int data_length = BLOCK_SIZE_FOR_DATA; // length for packet data: [0,1472 - 8]
  if(packet_total_numbers -1 == sequenceNumber && last_packet_size != 0){
      data_length = last_packet_size;
  }

  //gabage init value
  char * data = new char[data_length];
  memset(data, 0, data_length);

  time(&timer);
  long int timestamp = timer;

  int byteRead = fread(data, sizeof(char), data_length, fp);
  if(byteRead < data_length){
    diep("read failure, expected for more byte. Check if your byte to transfer matches your file size. ");
    exit(1);
  }

  Packet * new_packet = new Packet(sequenceNumber, timestamp, data_length, data);
  return new_packet;
}

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

    // struct sockaddr_in si_other;
    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    /* decimal ip addr to network bigendian addr*/
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
	/* Send data and receive acknowledgements on s*/
    init(bytesToTransfer);

    /* sending buffer and receiving buffer */
    char current_msg[BLOCK_SIZE_FOR_DATA]; // gabage initial value
    memset(current_msg, 0, BLOCK_SIZE_FOR_DATA);
    char ack_msg_buf[BLOCK_SIZE_FOR_HEADER];
    memset(ack_msg_buf, 0, BLOCK_SIZE_FOR_HEADER);

    /* threading TBD */
    // TODO


    int byteReceived = 0;
    int byteSent = 0;
    unsigned long long int received_ack_number;
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    memset(sockaddr_in, 0, sizeof(sockaddr_in));

    first_recv = 1;
    while(1){
      //TODO: if we should recv any udp pakcet, if recvfrom needs length spec
      if(first_recv ==1 ){
      byteReceived = recefrom(s, ack_msg_buf, sizeof(ack_msg_buf), 0, (sockaddr *)&addr, &addr_size);
      if(byteReceived != sizeof(ack_msg_buf)){
        perror("incorrect data format, size of ack data should be 8 bytes");
        exit(1);
      }
      first_recv = 0;
      memcpy(&received_ack_number, ack_msg_buf, sizeof(unsigned long long int));
      cout << "firest_recv_ack: "<< endl;
      printBits(8, received_ack_number);
    }

    // when ack is larger than current
    Packet * current_packet = prepareData(fp, current_sequenceNumber);
    memcpy(current_msg, &(current_packet->sequenceNumber), sizeof(unsigned long long int));
    memcpy(current_msg + 12, current_packet->data, current_packet->length);
    byteSent = sendto(s, current_msg, current_packet->length, 0, (sockaddr*)&si_other, slen);
    if(current_sequenceNumber == packet_total_numbers){
      break;
    }else{
      current_sequenceNumber++;
    }
  }
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

    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);


    return (EXIT_SUCCESS);
}
