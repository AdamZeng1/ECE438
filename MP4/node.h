#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

class Node
{
public:
  int index;
  int backoff;
  int colisionNum;
  Node(int index);
};

#endif