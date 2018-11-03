#include"test_obj.h"
#include<iostream>
#include<stdlib.h>
#include<cstring>

using namespace std;

int main(int argc, char * argv[]){

  if (argc != 1){
    cout << "there shouldn't be any arguments except the name of the file";
  }

  char data[20] = {"abcndabcndabcndabcn"};
  // memset(data, 0, sizeof data);
  long sequenceNumber = 72L;
  int receiveWindow = 72;
  // char temp[] = "Suprise";

  // strncpy(data, temp, 7);

  Packet * test = new Packet(sequenceNumber, receiveWindow, data);
  // test->setSequenceNumber(sequenceNumber);
  // test->setData(data);
  // test->setReceiveWindow(receiveWindow);


  char current_msg[20] = {};
  char * shift = current_msg + 4;
  int addr = (long) current_msg;
  int addr_shift = (long) shift;
  // char * current_msg = (char*)malloc(30);
  // memset(current_msg, 0, 30);
  memcpy(current_msg, &(test->sequenceNumber),4 );
  memcpy(current_msg + 4, &(test->receiveWindow), sizeof(int));
  cout << "this is current_msg: " << current_msg << endl;
  cout << "this is current_msg + 4: " <<  shift << endl;
  cout << "this is addr of current_msg: " << addr << " and shift: " << addr_shift << endl;
  //
  // cout << "this is the seq num: " << test->getSequenceNumber();
  // cout << endl;
  // cout << "this is the data: " << test->getData();
  // cout << endl;
  // cout << "this is the rec win: "<< test->getReceiveWindow();
  // cout << endl;

  return 0;
}
