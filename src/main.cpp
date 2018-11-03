#include"test_obj.h"
#include<iostream>
#include<stdlib.h>
#include<cstring>
#include<typeinfo>
#include<ctime>

#define TEST 26
using namespace std;

int test_macro;

int main(int argc, char * argv[]){

  if (argc != 1){
    cout << "there shouldn't be any arguments except the name of the file";
  }



  char data[20] = {"abcndabcndabcndabcn"};
  // memset(data, 0, sizeof data);
  long sequenceNumber = 72L;
  int receiveWindow = 73;

  time_t timer;
  // struct tm y2k = {0};
  // y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
  // y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
  // time_t std_time = mktime(&y2k);
  // time(&timer);  /* get current time; same as: timer = time(NULL)  */
  // double t = difftime(timer,std_time);
  time(&timer);
  long int t = timer;

  Packet * test = new Packet(sequenceNumber, t, 1500, data);
  // test->setSequenceNumber(sequenceNumber);
  // test->setData(data);
  // test->setReceiveWindow(receiveWindow);


  // test_macro = TEST;
  // cout << "this is test for macro:  " << test_macro << "   type of macro: " << typeid(test).name() << endl;



  // cout << timer << "    <-- this is the current time" << endl;
  cout << test->sentTime << "   <-- this is the time saved in the class, and size of long :  " << sizeof(long int) << endl;
  // cout << typeid(timer).name() << "   <--- this is the type of time, and size of timer: "<< sizeof(timer) << endl;



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

  for(int i = 0; i < sizeof(current_msg); i++){
    cout << current_msg[i] << endl;
  }

  // cout << "this is the seq num: " << test->getSequenceNumber();
  // cout << endl;
  // cout << "this is the data: " << test->getData();
  // cout << endl;
  // cout << "this is the rec win: "<< test->getReceiveWindow();
  // cout << endl;

  return 0;
}
