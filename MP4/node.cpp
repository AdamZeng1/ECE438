#include "node.h"
using namespace std;

Node::Node(int index)
{
  this->index = index;
  this->backoff = 0;
  this->colisionNum = 0;
}