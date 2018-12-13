#include "node.h"
#include <stdlib.h>
using namespace std;

Node::Node(int index, int maximalBackoff)
{
  this->index = index;
  this->backoff = 0;
  this->colisionNum = 0;
  this->maximalBackoff = maximalBackoff;
}

void Node::setRandom()
{
  srand(time(NULL));
  this->backoff = rand() % maximalBackoff + 1;
}