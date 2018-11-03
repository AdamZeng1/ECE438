#include"test_obj.h"
using namespace std;

Packet::Packet(){
  this->receiveWindow = 1;
  this->sequenceNumber = 2;
}

Packet::Packet(long seq, int rec, char dat[]){
  this->sequenceNumber = seq;
  this->receiveWindow = rec;
  this->data = dat;
}

long Packet::getSequenceNumber(){
  return this->sequenceNumber;
}

int Packet::getReceiveWindow(){
  return this->receiveWindow;
}

// int Packet::getLength(){
//   return this->length;
// }

char * Packet::getData(){
  return this->data;
}

void Packet::setReceiveWindow(int rec){
  this->receiveWindow = rec;
}

void Packet::setSequenceNumber(long seq){
  this->sequenceNumber = seq;
}

void Packet::setData(char dat[]){
  this->data = dat;
}
