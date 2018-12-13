#include "node.h"
#include <stdlib.h>
#include <ctime>
#include <vector>
using namespace std;

Node::Node(int index, int maximalBackoff)
{
  this->index = index;
  this->backoff = 0;
  this->colisionNum = 0;
  this->maximalBackoff = maximalBackoff;
}

void Node::setRandom(vector<int>& R)
{
  this->backoff = rand() % (R[this->colisionNum]) + 1;
}
