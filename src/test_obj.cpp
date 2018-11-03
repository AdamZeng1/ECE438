#include"test_obj.h"
using namespace std;

Packet::Packet(){
  this->receiveWindow = 1;
  this->sequenceNumber = 2;
}

Packet::Packet(unsigned long long int seq, long int t, int length, char dat[]){
  this->sequenceNumber = seq;
  this->sentTime = t;
  this->data = dat;
  this->length = length;
}

// long Packet::getSequenceNumber(){
//   return this->sequenceNumber;
// }
//
// int Packet::getReceiveWindow(){
//   return this->receiveWindow;
// }

// int Packet::getLength(){
//   return this->length;
// }
//
// char * Packet::getData(){
//   return this->data;
// }

void Packet::setReceiveWindow(int rec){
  this->receiveWindow = rec;
}

void Packet::setSequenceNumber(unsigned long long int seq){
  this->sequenceNumber = seq;
}

void Packet::setData(char dat[]){
  this->data = dat;
}

void Packet::setSentTime(long int t){
  this->sentTime = t;
}
