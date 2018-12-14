#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

class Node
{
public:
  int index;
  int backoff;
  int colisionNum;
  int maximalBackoff;
  int total_col;
  int total_trans;
  Node(int index, int maximalBackoff);
  void setRandom(vector<int> &R);
};

#endif
