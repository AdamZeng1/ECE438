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
    long sequenceNumber;
    char * data;
    int receiveWindow;
    int getReceiveWindow();
    char * getData();
    // int getLength();
    long getSequenceNumber();

    Packet();
    Packet(long seq, int rec, char dat[]);

    void setReceiveWindow(int rec);
    void setSequenceNumber(long seq);
    void setData(char dat[]);

};

#endif
