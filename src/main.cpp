#include"test_obj.h"
#include<iostream>
#include<stdlib.h>
#include<cstring>
#include<typeinfo>
#include<ctime>
#include<string.h>
#include<stdio.h>
#include<stdint.h>

#define TEST 26
using namespace std;

int test_macro;

void printBits(size_t const size, void const * const ptr){
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

int main(int argc, char * argv[]){

  if (argc != 1){
    cout << "there shouldn't be any arguments except the name of the file";
  }

  unsigned long long int  sequenceNumber = 256L;
  int receiveWindow = 73;

  time_t timer;
  // struct tm y2k = {0};
  // y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
  // y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
  // time_t std_time = mktime(&y2k);
  // time(&timer);  /* get current time; same as: timer = time(NULL)  */
  // double t = difftime(timer,std_time);
  time(&timer);
  int t = timer;
  char* data = "alice and bob ";
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
  char * shift = "yeah";

  char * tt = "hey";

  char * ttt[20];
  memset(ttt, 0, 20);
  ttt[0] = shift;
  ttt[1] = tt;
  cout << "size " << sizeof(ttt[0]) << " and " << sizeof(ttt[1]) << endl;
  cout << "size " << ttt[0] << " and " << ttt[1]<< endl;

  // cout << sizeof(shift) << "  <- ptr size" << endl;
  memset(current_msg, 0, sizeof(unsigned long long int));


  unsigned long long int z = 42949689406372972899L;
  cout << "long int " << z << endl;
  uint32_t m = (uint32_t) z;
  cout << "unsigned int " << m << endl;
  // memcpy(current_msg, &(sequenceNumber), sizeof(unsigned long long int) );
  // memcpy(current_msg, )
  // memcpy(shift, &(test->receiveWindow), sizeof(int));
  // memcpy(&z, tt[0], 8);
  // cout << "success: ? " << z << endl;
  cout << "this is current_msg: " << current_msg << endl;
  // for(int i = 0; i < sizeof(unsigned long long int); i++){
  //   cout << "the " << i << "th char: " << current_msg[i] << endl;
  // }
  //

  // printBits(8, t);
  // cout << "latter 8 bits: " << endl;
  // printBits(8, t + 8);
  // cout << "this is current_msg + 8: " <<  shift << endl;
  // cout << "test_> seq: " << test->sequenceNumber << endl;
  // cout << "this is addr of current_msg: " << addr << " and shift: " << addr_shift << endl;
  string tmp = "";
  tmp.append("failed in sending the packet");
  tmp.append(to_string(z));
  cout << tmp.c_str() << endl;

  cout << "type of :" << sizeof(14134324L * 21434234) << endl;

  return 0;
}
