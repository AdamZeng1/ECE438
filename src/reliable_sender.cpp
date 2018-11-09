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
#define INIT_SST  40
#define INIT_CWND_START  0
#define INIT_CWND_SIZE  1
#define BLOCK_SIZE_FOR_HEADER  12

//Byte 1472 - 8 MTU = 1500 udp_header + ipheader = 28; for 1 GB = 10e9B = 10e6 pkt;
#define BLOCK_SIZE_FOR_DATA  1460
#define PACKET_BUFFER_SIZE  1000
#define INIT_TIMEOUT_SEC
#define INIT_TIMEOUT_USEC

// -10:   drop rate=0.05 failed: received file had an error.
// -10:   drop rate=0.25 failed: only received 0 of 10000 after 10s.
//  quick convergence succeeded!
//  TCP friendliness succeeded! (TCP got 36.0 Mbps)
// -10:  TCP friendliness with 1% loss failed: your process died or ended early. (note: it's being given 999000999000 as the numBytes argument)
//  competition with tcp succeed
//TODO:
// timeout - packet loss
// wait_flag update
// current_sequenceNumber update
// when incoming ack is far over your expectation when you recovered from FR


// c++ library and Packet class
#include"test_obj.h"
#include<iostream>
#include<stdint.h>
#include<string>
// #include<unistd.h>
#include<errno.h>
using namespace std;

char LOGBUFF[1024];

int s, slen; // slen: the length of sockaddr_in, s:socket
struct sockaddr_in si_other;

// int current_transmit_round; // set to be 1 when init, plus one if circlely using the packet_window
// int total_round;
unsigned int cwnd_start, cwnd_end; // set to be INIT_CWND_START
unsigned int ss_threshold, cwnd_size; // set to be INIT_SST, 1
bool wait_flag; // set to be false when init
bool timeout_flag; // set to be false when init
bool transmission_finished_flag; // set to be false when int

Packet *  packet_window[PACKET_BUFFER_SIZE];

unsigned long long int packet_total_numbers; // total number of packets = total_bytes / 1464
unsigned int last_packet_size;
struct timeval tv;

void diep(char *s) {
    perror(s);
    exit(1);
}

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
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

/* return current time in long using glbal timeval tv*/
long long int currentTime_ms(void){
  gettimeofday(&tv, NULL);
  long long int rst;
  rst = (long long int)tv.tv_sec * 1000 + ((long long int)tv.tv_usec) / 1000;
  return rst;
}

void init(unsigned long long int numberBytes){
  cwnd_start = INIT_CWND_START;
  cwnd_size = INIT_CWND_SIZE;
  cwnd_end = cwnd_start + cwnd_size;
  ss_threshold = INIT_SST;

  wait_flag = false;
  timeout_flag = false;
  transmission_finished_flag = false;

  last_packet_size = numberBytes % BLOCK_SIZE_FOR_DATA;
  if(last_packet_size == 0){
    packet_total_numbers = numberBytes / BLOCK_SIZE_FOR_DATA;
  }else{
    packet_total_numbers = numberBytes / BLOCK_SIZE_FOR_DATA + 1;
  }
  cout << "total packet numbers: " << packet_total_numbers << "    last_packet_size:  " << last_packet_size << endl;
  memset(packet_window, 0, sizeof(packet_window));
  cout << "memset for pkt buf: " << sizeof(packet_window) << endl;
  // if(rd == 0){
  //   total_round = packet_total_numbers / PACKET_BUFFER_SIZE;
  // }else{
  //   total_round = packet_total_numbers / PACKET_BUFFER_SIZE + 1;
  // }

  memset(LOGBUFF, 0, 1024);
}

/* prepare file data into Packets, append timestamp info */
Packet* prepareData(FILE * fp, unsigned long long int sequenceNumber){
  unsigned int data_length = BLOCK_SIZE_FOR_DATA; // length for packet data: [0,1472 - 8]
  if(packet_total_numbers -1 == sequenceNumber && last_packet_size != 0){
      data_length = last_packet_size;
  }

  //gabage init value
  char * data = new char[data_length];
  memset(data, 0, data_length);

  long long int timestamp;
  timestamp = currentTime_ms();

  int byteRead = fread(data, sizeof(char), data_length, fp);
  if(byteRead < data_length){
    diep("read failure, expected for more byte. Check if your byte to transfer matches your file size. ");
    exit(1);
  }

  Packet * new_packet = new Packet(sequenceNumber, timestamp, data_length, data);
  return new_packet;
}

int send_msg(char * current_msg, Packet * packet_to_sent){
  memcpy(current_msg, &(packet_to_sent->sequenceNumber), sizeof(unsigned long long int));
  memcpy(current_msg + 12, packet_to_sent->data, packet_to_sent->length);

  /* test for packet obj before sent*/
  // cout << "this is a test for sending out the first packet: " <<  current_packet->sequenceNumber << "  this is for packet data: " << current_packet->data << endl;
  // cout << "length: " << current_packet->length << endl;
  // cout << "timestamp: " << current_packet->sentTime << endl;
  /* test for var */
  // cout << " packet_total_numbers:" << packet_total_numbers << endl; // total number of packets = total_bytes / 1464
  // cout << " last_packet_size:" << last_packet_size << endl;
  int sentByte = sendto(s, current_msg, packet_to_sent->length + BLOCK_SIZE_FOR_HEADER, 0, (sockaddr*)&si_other, slen);
  return sentByte;
}

/* main function  */
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

    /* error message, no need for modern libc*/
    // extern int errno;

    /* state of transmit PKT and receive ACK*/
    // use garbage ack number for init
    unsigned long long int lastAckedNum = 1000L;
    unsigned long long int expected_ack_number = 0;
    unsigned long long int current_sequenceNumber = 0;
    int dupAckCount = 0;
    bool isSlowStart = true;
    bool isSST = false;
    bool isFastRecovery = false;
    int countAfterThreshold = 0;
    int sentByte = 0;
    Packet * current_packet;

    /*timeout init setting*/
    int sockStateSet = 0;
    struct timeval timeout_struct;
    struct timeval round_trip_time;
    memset(&timeout_struct, 0, sizeof(timeout_struct));
    memset(&round_trip_time, 0, sizeof(timeout_struct));
    unsigned long long int current_time;
    bool isFirstPacket = true;
    // bool isFirstAck = false;
    unsigned long long int timer_cwnd_starter = 0L;
    unsigned int timeout_ms = 0; // only metric for timeout of certain packet


    /* sending buffer and receiving buffer */
    char current_msg[BLOCK_SIZE_FOR_DATA]; // gabage initial value
    memset(current_msg, 0, BLOCK_SIZE_FOR_DATA);
    char ack_msg_buf[BLOCK_SIZE_FOR_HEADER];
    memset(ack_msg_buf, 0, BLOCK_SIZE_FOR_HEADER);


    /* threading TBD */
    // TODO: used to checkout the timeout threading

    /* receiving params */
    int byteReceived = 0;
    unsigned long long int received_ack_number;
    unsigned int receiver_window; // received but not currently in use
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);// container for recvfrom

    // all false --flag-- when init
    while(!transmission_finished_flag){

      /* TODO setsockopt*/


      while(wait_flag){
        //TODO:
        // if we should recv any udp pakcet, if recvfrom needs length spec
        cout << "###DEBUG### start listening for: ACK" << expected_ack_number << endl;


        if(isFirstPacket){
          // TODO, fixed timeout, measured from the first packet.
          // timeout_struct; round_trip_time;
          timeout_struct.tv_sec = round_trip_time.tv_sec;
          timeout_struct.tv_usec = round_trip_time.tv_usec;

          // milliseconds
          struct timeval * opt = &timeout_struct;
          if(expected_ack_number == 0){
            timeout_struct.tv_sec = 0;
            timeout_struct.tv_usec = 40000;
          }
          timeout_ms = (unsigned int) (timeout_struct.tv_usec / 1000);
          cout << "init timeout_ms: " << timeout_ms << " ms" << endl;
          cout << "expected_ack  " << expected_ack_number << "  tv_sec-"  << timeout_struct.tv_sec << "  tv_usec-" << timeout_struct.tv_usec << endl;
          // isFirstPacket = false;
        }


        sockStateSet = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &(timeout_struct), sizeof(timeout_struct));
        if(sockStateSet < 0 ) {
          char * mesg = strerror(errno);
          cout << "set sock opt failed "<< "errorcode-" << errno << "  " << mesg << endl;
        }
        int sockopt_val = recvfrom(s, ack_msg_buf, sizeof(ack_msg_buf), 0, (sockaddr *)&addr, &addr_size);


        timer_cwnd_starter = currentTime_ms() - (packet_window[cwnd_start])->sentTime;
        if(timer_cwnd_starter >= timeout_ms){
          timeout_flag = true;
        }
        cout << "timer for packet number " << packet_window[cwnd_start]->sequenceNumber << "   is: "<<  timer_cwnd_starter << endl;

        /* first cond alone only works for first round timeout or possibly last single packet */
        if(sockopt_val < 0 || timeout_flag){
          cout << "sockopt_val-"<<sockopt_val  <<  "   timeout_flag-" << timeout_flag <<   "   timer_cwnd_starter-"<< timer_cwnd_starter <<  endl;

          if(sockopt_val < 0){
            char * mesg = strerror(errno);
            cout << "error occurred, the messages listed below: " << "errorcode-" << errno << "  " << mesg << endl;
          }
          /* timeout handler */
          Packet * resend_pkt = packet_window[cwnd_start];
          current_time= currentTime_ms();
          // error occurs
          resend_pkt->sentTime = current_time;
          int byteSent = send_msg(current_msg, resend_pkt);
          if(byteSent <= 0){
            perror("error in resending for timeout handler");
          }
          cout << "resend packet: bytessent-" << byteSent  << "  seqNum-" << resend_pkt->sequenceNumber << endl;
          ss_threshold = cwnd_size /2;
          cwnd_size = 1;
          dupAckCount = 0;
          if(ss_threshold < 1){ss_threshold = 1;}
          cwnd_end = cwnd_start + cwnd_size;
          isSlowStart = true;
          isSST = false;
          timeout_flag = false;
          continue;
        }

          // retrieve packet from sysbuf
          // cout << "###DEBUG### recv from worked: got bytes " << byteReceived << endl;
        byteReceived = sockopt_val;
        if(byteReceived == 0){cout << "receive 0 bytes from the sysbuf" << endl;}

        /* receive value parser */
        sscanf(ack_msg_buf, "%lld", &received_ack_number);
        // memcpy(&received_ack_number, ack_msg_buf, sizeof(unsigned long long int));
        // memcpy(&receiver_window, ack_msg_buf + sizeof(unsigned long long int), sizeof(unsigned int));

        /* when ack is larger than current */
        if(received_ack_number >= expected_ack_number){

          /* check for final of transmit */
          if(received_ack_number == packet_total_numbers - 1){
            transmission_finished_flag = true;
            cout << "###FIN### received new ack;   ack_number: " <<received_ack_number << "  current_sequenceNumber: "<< current_sequenceNumber;
            cout << "cwnd(not update last time):  start-" << cwnd_start << " size-" << cwnd_size << endl;
            break;
          }

          // downgrade to uint32 since the max of differences will be cwnd_size;
          unsigned int ack_diff = (received_ack_number - expected_ack_number + 1);
          unsigned int rd_mod = (received_ack_number + 1) % PACKET_BUFFER_SIZE;
          unsigned int rd = (received_ack_number + 1) / PACKET_BUFFER_SIZE;

          /* update the round trip time history*/
          if(isFirstPacket){
          current_time = currentTime_ms();
          Packet * legacy_packet = packet_window[INIT_CWND_START];
          // double the real time timeout
          round_trip_time.tv_sec =  (current_time - legacy_packet->sentTime) / 500;
          round_trip_time.tv_usec = ((current_time - legacy_packet->sentTime) % 1000) * 2000 ;
          cout << "update round time after the first ack:  tv_usec-" << round_trip_time.tv_usec << endl;
          isFirstPacket = false;
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

          if(isFastRecovery){
            // TODO: react after dupACK * 3
            cwnd_size = ss_threshold;
            //
            if(ack_diff > 1){
              // when ackdiff + start
              countAfterThreshold = ack_diff - 1;
              while(countAfterThreshold >= cwnd_size){
                cwnd_size++;
                countAfterThreshold = (countAfterThreshold) % cwnd_size;
              }
            }
            isSST = true;
            isFastRecovery = false;
            dupAckCount = 0;
            cout << "exit fast recovery state " << "  cwnd_size"  << cwnd_size <<endl;
            //DEBUG temp
            current_sequenceNumber = received_ack_number + 1;
          }
          wait_flag = false;

          /* uniformly update cwnd pointer */
          // start = 8;
          // end =
          // expect = 9;

          cwnd_start = rd_mod;
          cwnd_end = (cwnd_start + cwnd_size) % PACKET_BUFFER_SIZE;
          lastAckedNum = received_ack_number;
          expected_ack_number = received_ack_number + 1;
          cout << "###DEBUG### receiving new ack-" << received_ack_number << "  afterwards, we get cwnd size " << cwnd_size <<  "start: " << cwnd_start  << endl;


          // TODO TODO:
          // error reproduce: when it exits from fast recovery state, it will try to listening to an ack which will not be received(since the associated packet has never been sent) and some object(PACKET maybe, not created that time) will be accessed, causing segfault.
          // easy to reproduce
          // possible bug: cwnd_end, current_sequenceNumber wrongly updated when state(FR enter, FR exit, SST enter, SST exit, ) changes

        }else{
          // TODO: Fast Recovery or outdate ack
          // received ack number <(may be wrap around) expected number, outdated(should be ignored) or dupack()
          if (lastAckedNum == received_ack_number){
            dupAckCount ++;
            cout << "###DEBUG### dup -" << received_ack_number << "  afterwards, we get cwnd size " << cwnd_size <<  "start: " << cwnd_start  << endl;

            if(dupAckCount == 3 && !isFastRecovery){
              cout << "dupack number == 3, start fast recovery" << endl;
              isSlowStart = false;
              isFastRecovery = true;
              ss_threshold = cwnd_size /2;
              if(ss_threshold < 1){ss_threshold = 1;}
              cwnd_size = ss_threshold + 3;
              cwnd_end = cwnd_start + cwnd_size;
              /* TODO careful with this, since the file cannot be applied with fseek()*/
              current_sequenceNumber = cwnd_end;
              // TODO: resend the missing packet
              Packet * resend_packet = packet_window[cwnd_start];
              // current_time= currentTime_ms();
              // resend_packet->sentTime = current_time;
              int sentByte = send_msg(current_msg, resend_packet);

              if(sentByte <= 0){
                perror("resend failed after 3 dupACK");
              }
              cout << "resent pkt sequenceNumber: " << resend_packet->sequenceNumber << endl;
              wait_flag = true;
            }
            if(dupAckCount > 3 && isFastRecovery){
              //update cwnd, and
              cwnd_size++;
              cwnd_end = (cwnd_start+cwnd_size)%PACKET_BUFFER_SIZE;
              current_sequenceNumber = cwnd_end-1;
              // Packet * resend_packet = packet_window[cwnd_start];
              // current_time= currentTime_ms();
              // resend_packet->sentTime = current_time;
              // int sentByte = send_msg(current_msg, resend_packet);
              // cout << "resend for " << dupAckCount - 3 <<  " times for the fast recovery " << endl;

              break;
            }
          } else{
            cout << "outdated ack: " << received_ack_number << "will be discarded..." << endl;
            continue; } // else lastAckNumber outdated}
        }
      }/* WAIT FLAG END HERE */

      // when the dupAck is piled up to 3, if A. cw == 1
      if(dupAckCount == 3 && isFastRecovery){
        continue;
      }

      /* in case of intended to send only part of the file, with spare cwnd*/
      if(current_sequenceNumber > packet_total_numbers -1 || (current_sequenceNumber % PACKET_BUFFER_SIZE) >= cwnd_end){
        wait_flag = true;
        continue;
      }

      /* prepare the next packet and replace delete the packet who was in that place */
      unsigned int cwnd_current_pos = (current_sequenceNumber) % PACKET_BUFFER_SIZE;
      if ((packet_window[cwnd_current_pos]) != NULL && (packet_window[cwnd_current_pos]->sequenceNumber) == current_sequenceNumber){
        current_packet = packet_window[cwnd_current_pos];
        current_packet->sentTime = currentTime_ms();
      }else{
        current_packet =  prepareData(fp, current_sequenceNumber);
        if (packet_window[cwnd_current_pos] != NULL){
          delete(packet_window[cwnd_current_pos]);
        }
        packet_window[cwnd_current_pos] = current_packet;
      }

      /* send buffer */
      int sentByte = send_msg(current_msg, current_packet);

      if(isFastRecovery) {
        cout << "###DEBUG### fast recovery extended sending, sequence number: " << current_sequenceNumber << "  sent byte(data+head): " << sentByte << " sent byte(head): " << BLOCK_SIZE_FOR_HEADER <<endl;
      }else{
      cout << "###DEBUG### sequence number: " << current_sequenceNumber << "  sent byte(data+head): " << sentByte << " sent byte(head): " << BLOCK_SIZE_FOR_HEADER <<endl;
      // fputs(LOGBUFF, fp);
      }

      if(sentByte ==  -1 || sentByte < current_packet->length){
        cout << "failed in sending the packet Num. " << current_sequenceNumber << ". Have sent " << sentByte << " in this transmission. ";
        diep("shut down for not successfully send out a packet! ");
      }

      // /*TODO: if it is the first packet, then set the timeout to be the round_trip_time */
      // if(isFirstPacket){
      //   // TODO, fixed timeout, measured from the first packet.
      //   // timeout_struct; round_trip_time;
      //   timeout_struct.tv_sec = round_trip_time.tv_sec;
      //   timeout_struct.tv_usec = round_trip_time.tv_usec;
      //   unsigned int timeout_ms = (unsigned int) (timeout_struct.tv_usec / 1000);
      //   // milliseconds
      //   struct timeval * opt = &timeout_struct;
      //   if(expected_ack_number == 0){
      //     timeout_struct.tv_sec = 0;
      //     timeout_struct.tv_usec = 40000;
      //   }
      //   socklen_t optlen = (unsigned int) sizeof(timeout_struct);
      //   int level = SOL_SOCKET;
      //   int optname = SO_SNDTIMEO;
      //   sockStateSet = setsockopt(s, level, SO_RCVTIMEO, &(timeout_struct), optlen);
      //   isFirstPacket = false;
      //   if(sockStateSet < 0){
      //     perror("cannot set sockopt");
      //   }
      //   cout << "timeout settled: " << timeout_struct->tv_usec << endl;
      // }


      current_sequenceNumber++;
      if( (current_sequenceNumber % PACKET_BUFFER_SIZE) < cwnd_end ){
        continue;
      }else{
        wait_flag = true;
      }

      /* TODO: */
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
