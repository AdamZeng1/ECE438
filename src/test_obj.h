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
    uint32_t sequenceNumber;
    char * data;
    int receiveWindow;
    long int sentTime;
    int length;
    // int getReceiveWindow();
    // char * getData();
    // int getLength();
    // long getSequenceNumber();

    Packet();
    Packet(uint32_t seq, long int t, int length, char dat[]);

    void setReceiveWindow(int rec);
    void setSequenceNumber(uint32_t seq);
    void setData(char dat[]);
    void setSentTime(long int t);

};

#endif
