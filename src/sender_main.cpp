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
#define BLOCK_SIZE_FOR_HEADER  8
//Byte 1472 - 8 MTU = 1500 udp_header + ipheader = 28; for 1 GB = 10e9B = 10e6 pkt;
#define BLOCK_SIZE_FOR_DATA  1464
#define PACKET_BUFFER_SIZE  1000

// c++ library and Packet class
#include"test_obj.h"
/*
class Packet
{
  private:
    char * data;
    int receiveWindow;
    int length;
  public:
    long sequenceNumber;
    int getReceiveWindow();
    char * getData();
    int getLength();
    long getSequenceNumber();
    Packet();
    Packet(long seq, int rec, char dat[]);
    void setReceiveWindow(int rec);
    void setSequenceNumber(long seq);
    void setData(char dat[]);
};
*/
#include<iostream>

using namespace std;


int s, slen; // slen: the length of sockaddr_in, s:socket
struct sockaddr_in si_other;

int cwnd_start, cwnd_end; // set to be INIT_CWND_START
int ss_threshold, cwnd_size; // set to be INIT_SST, 1
bool wait_flag; // set to be false when init
bool timeout_flag; // set to be false when init
bool transmission_finished_flag; // set to be false when int

Packet * packet_window[PACKET_BUFFER_SIZE];

int transmit_round; // set to be 1 when init, plus one if circlely using the packet_window
int expected_round;
long packet_total_numbers; // total number of packets = total_bytes / 1464
int last_packet_size;
long int current_time;
time_t timer;

void diep(char *s) {
    perror(s);
    exit(1);
}

void init(int numberBytes){
  cwnd_start = INIT_CWND_START;
  cwnd_size = INIT_CWND_SIZE;
  cwnd_end = cwnd_start + cwnd_size;
  ss_threshold = INIT_SST;

  wait_flag = false;
  timeout_flag = false;
  transmission_finished_flag = false;

  // packet_window = {};
  transmit_round = 1;

  last_packet_size = numberBytes % BLOCK_SIZE_FOR_DATA;
  if(last_packet_size == 0){
    packet_total_numbers = numberBytes / BLOCK_SIZE_FOR_DATA;
  }else{
    packet_total_numbers = numberBytes / BLOCK_SIZE_FOR_DATA + 1;
  }

  int rd = packet_total_numbers % PACKET_BUFFER_SIZE;
  if(rd == 0){
    expected_round = packet_total_numbers / PACKET_BUFFER_SIZE;
  }else{
    expected_round = packet_total_numbers / PACKET_BUFFER_SIZE + 1;
  }

}

// prepare file data into Packets
//
Packet* prepareData(FILE * fp, long sequenceNumber){
  int length;
  if(packet_total_numbers -1 == sequenceNumber){
    if(last_packet_size == 0){
      length = BLOCK_SIZE_FOR_DATA;}
    else{
      length = last_packet_size;}
  }else{
    length = BLOCK_SIZE_FOR_DATA;
  }

  char data[BLOCK_SIZE_FOR_DATA] = {};
  // char* data;
  // data = (char *)malloc(length);

  time(&timer);
  long int timestamp = timer;

  fread(data, sizeof(char), length, fp);
  Packet * new_packet;
  if(length < BLOCK_SIZE_FOR_DATA){
    new_packet = new Packet(sequenceNumber, timestamp,BLOCK_SIZE_FOR_DATA, data);
  }else{
    new_packet = new Packet(sequenceNumber, timestamp, length, data);
  }

  return new_packet;
}

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    // char* hostname
    // unsigned short int hostUDPport
    // char* filename
    //unsigned long long int bytesToTransfer

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
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

	/* Send data and receive acknowledgements on s*/
    init(bytesToTransfer);

    long current_sequenceNumber = 0;
    char current_msg[BLOCK_SIZE_FOR_DATA];
    // threading for acknowledgements
    // threading for timeout monitor

    // while(!transmission_finished_flag){
      // while(wait_flag){
      //   sleep();
      // }
      Packet * current_packet = prepareData(fp, current_sequenceNumber);
      memcpy(current_msg, &(current_packet->sequenceNumber), sizeof(long));
      memcpy(current_msg + 4, &(current_packet->data), current_packet->length);
      cout << "this is a test for sending out the first packet: " <<  current_packet->sequenceNumber << "  this is for packet data: " << current_packet->data << endl;
      // cout << "\n" << endl;
      // char * tmp = current_msg + 4;
      // printBits(5, tmp);
      for(int i =0; i < 10; i++){
        cout << "this is " << i << "th char in data: " << current_msg[4+i] << endl;
      }

      char temp[10];
      memcpy(temp, current_msg + 4, 6);
      string str = temp;
      cout << str << endl;

      // sendto(s, current_msg, current_packet->length, 0, (struct sockaddr*)&si_other, slen);
    // }

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
