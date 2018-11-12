#ifndef TEST_OBJ_H
#define TEST_OBJ_H

#include <iostream>
#include <stdlib.h>
#include <string>

#define HEAD "THIS IS TEST.H"
using namespace std;

class Packet
{
    //time
    // int length;
  public:
    unsigned long long int sequenceNumber;
    char * data;
    int receiveWindow;
    long long int sentTime;
    int length;
    // int getReceiveWindow();
    // char * getData();
    // int getLength();
    // long getSequenceNumber();

    Packet();
    Packet(unsigned long long int seq, long long int t, int length, char dat[]);

    void setReceiveWindow(int rec);
    void setSequenceNumber(unsigned long long int seq);
    void setData(char dat[]);
    void setSentTime(long long int t);

};

#endif
