#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>
#include <set>

using namespace std;

class Node
{
public:
  int label;
  map<int, int> neighbors;
  map<int, int> distance_vector; // document all the neighbors
  map<int, int> next_hop;
  // should contain the cost of the
  bool updated;
  Node(int label);
};

#endif
