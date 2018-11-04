/*
 * File:   sender_main.c
 * Author:Xiaocong Yu & Keye Zhang
 *
 * Created on Nov.3 2018
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
#include<stdint.h>
#include<string>
using namespace std;

char LOGBUFF[1024];

int s, slen; // slen: the length of sockaddr_in, s:socket
struct sockaddr_in si_other;


int cwnd_start, cwnd_end; // set to be INIT_CWND_START
//
int ss_threshold, cwnd_size; // set to be INIT_SST, 1
bool wait_flag; // set to be false when init
bool timeout_flag; // set to be false when init
bool transmission_finished_flag; // set to be false when int

Packet *  packet_window[PACKET_BUFFER_SIZE];

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

void printBits(size_t const size, void const * const ptr)
{
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

  memset(LOGBUFF, 0, 1024);
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

    // logfileName = "logfile";
    // FILE *logfp;
    // logfp = fopen(logfileNamem 'a+');
    // if(logfp == NULL){
    //   printf("Could not open logfile to log.");
    //   exit(1);
    // }


    //TODO: verify the file size with the target bytesToTransfer

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

    // /* bind socket to receive from certain port */
    // int bindRet = bind(s, (sockaddr*)&si_other, sizeof(si_other));
    // TRACE("bindret = %d \r\n", bindRet);
    // TRACE("errcode = %d \r\n", WSAGetLastError());


	/* Send data and receive acknowledgements on s*/
    init(bytesToTransfer);

    /* state of transmit PKT and receive ACK*/
    unsigned long long int expected_ack_number = 0;
    unsigned long long int current_sequenceNumber = 0;
    int dupAckCount = 0;
    bool isSlowStart = true;
    bool isSST = false;
    bool isFastRecovery = false;
    int countAfterThreshold = 0;
    // use garbage ack number for init
    unsigned long long int lastAckedNum = 1000L;

    /* sending buffer and receiving buffer */
    char current_msg[BLOCK_SIZE_FOR_DATA]; // gabage initial value
    memset(current_msg, 0, BLOCK_SIZE_FOR_DATA);
    char ack_msg_buf[BLOCK_SIZE_FOR_HEADER];
    memset(ack_msg_buf, 0, BLOCK_SIZE_FOR_HEADER);
    bool ack_buf[PACKET_BUFFER_SIZE];


    /* threading TBD */
    // TODO: used to checkout the timeout

    /* receiving params */
    int byteReceived = 0;
    unsigned long long int received_ack_number;
    unsigned int receiver_window;
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);

    // all false --flag-- when init
    while(!transmission_finished_flag){

      while(wait_flag){
        //TODO:
        // if we should recv any udp pakcet, if recvfrom needs length spec
        cout << "###DEBUG### start listening for: ACK" << expected_ack_number << endl;
        byteReceived = recvfrom(s, ack_msg_buf, sizeof(ack_msg_buf), 0, (sockaddr *)&addr, &addr_size);
        cout << "###DEBUG### recv from worked: got bytes " << byteReceived << endl;

        if(byteReceived != sizeof(ack_msg_buf)){
          perror("incorrect data format, size of ack data should be 12 bytes");
          exit(1);
        }
        // when ack is larger than current
        memcpy(&received_ack_number, ack_msg_buf, sizeof(unsigned long long int));
        memcpy(&receiver_window, ack_msg_buf + sizeof(unsigned long long int), sizeof(unsigned int));
        if(received_ack_number >= expected_ack_number){

          /*last check for receiving*/
          if(received_ack_number == packet_total_numbers){
            transmission_finished_flag = true;
            break;
          }
          // downgrade to uint32 since the max of differences will be cwnd_size;
          unsigned int ack_diff = (received_ack_number - expected_ack_number + 1);
          unsigned int rd_mod = received_ack_number % PACKET_BUFFER_SIZE;
          unsigned int rd = received_ack_number / PACKET_BUFFER_SIZE;

          if(isFastRecovery){
            // TODO: react after dupACK * 3

          }
          if(isSST){
            // TODO: react when in SST
            if(ack_diff + countAfterThreshold >= cwnd_size){
              countAfterThreshold = ack_diff % cwnd_size;
              cwnd_size++;
            }else{
              countAfterThreshold += ack_diff;
            }
          }
          //round and expected round -> round to examine the
          if(isSlowStart){
            // update current cwnd_size/start/end ss_threshold
            if(ack_diff+cwnd_size >= ss_threshold){
              int diff = ss_threshold - cwnd_size;
              cwnd_size = ss_threshold;
              countAfterThreshold = ack_diff - diff;
              isSlowStart = false;
              isSST = true;
            }
            else{
              // final state is still in the SS other than SST
              cwnd_size = cwnd_size + ack_diff;
            }
          }
          /* uniformly update cwnd pointer */
          cwnd_start = rd_mod;
          cwnd_end = (cwnd_start + cwnd_size) % PACKET_BUFFER_SIZE;
          lastAckedNum = received_ack_number;

          wait_flag = false;
        }else{
          // TODO: Fast Recovery or outdate ack
          // received ack number <(may be wrap around) expected number, outdated(should be ignored) or dupack()
          if (lastAckedNum == received_ack_number){
            dupAckCount ++;
            if(dupAckCount == 3){
              isSlowStart = true;
              ss_threshold = ss_threshold /2;  // TODO: how to narrow down, lose precision
            }
          }
          continue;
        }


      }

      /* after while-loop for recv() an acknowledgement, cwnd_start and end have been updated*/

      Packet * current_packet = prepareData(fp, current_sequenceNumber);
      int cwnd_current_pos = current_sequenceNumber % PACKET_BUFFER_SIZE;
      if (packet_window[cwnd_current_pos] != NULL){
        delete(packet_window[cwnd_current_pos]);
      }
      packet_window[cwnd_current_pos] = current_packet;

      memcpy(current_msg, &(current_packet->sequenceNumber), sizeof(unsigned long long int));
      memcpy(current_msg + 12, current_packet->data, current_packet->length);

      /* test for packet obj*/
      // cout << "this is a test for sending out the first packet: " <<  current_packet->sequenceNumber << "  this is for packet data: " << current_packet->data << endl;
      // cout << "length: " << current_packet->length << endl;
      // cout << "timestamp: " << current_packet->sentTime << endl;

      // /* test for var */
      // cout << "current_transmit_round:" << current_transmit_round << endl; // set to be 1 when init, plus one if circlely using the packet_window
      // cout << " total_round:" << total_round << endl;
      // cout << " packet_total_numbers:" << packet_total_numbers << endl; // total number of packets = total_bytes / 1464
      // cout << " last_packet_size:" << last_packet_size << endl;
      int sentByte = 0;
      sentByte = sendto(s, current_msg, current_packet->length  + BLOCK_SIZE_FOR_HEADER, 0, (sockaddr*)&si_other, slen);
      cout << "###DEBUG### sequence number: " << current_sequenceNumber << "  sent byte(data+head): " << sentByte << " sent byte(head): " << BLOCK_SIZE_FOR_HEADER <<endl;
      // // buf
      // fputs(LOGBUFF, fp);


      if(sentByte ==  -1 || sentByte < current_packet->length){
        string tmp = "";
        cout << "failed in sending the packet Num. " << current_sequenceNumber << ". Have sent " << sentByte << " in this transmission. ";
      }

      current_sequenceNumber++;
      if( current_sequenceNumber < cwnd_end ){
        continue;
      }else{
        wait_flag = true;
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
