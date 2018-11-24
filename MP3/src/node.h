#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>

using namespace std;

class Node {
public:
    int label;
    map<int, int> neighbors;

    Node(int label);
};

#endif
