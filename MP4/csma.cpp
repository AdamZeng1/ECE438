#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <climits>
#include <algorithm>

using namespace std;

#include "node.h"
// // use ofstream to write documents
// ofstream outputFile("output.txt");

bool operator<(Node const &a, Node const &b)
{
  return a.backoff < b.backoff;
}

void split(const string &s, vector<int> &sv, const char flag = ' ')
{
  sv.clear();
  istringstream iss(s);
  string temp;

  while (getline(iss, temp, flag))
  {
    sv.push_back(stoi(temp));
  }
  return;
}

// second params newly added
void readFile(int *paramSet, string inputFile)
{
  ifstream in(inputFile); //be careful to the real file path

  if (!in)
  {
    cout << "Cannot open input file.\n";
    return;
  }

  char str[255];
  int index = 0;
  while (in)
  {
    in.getline(str, 255); // delim defaults to '\n'
    if (in)
      cout << str << endl;

    //split line
    vector<int> sv; // node1, node2, distance
    split(str, sv, ' ');
    if (sv.size() == 0)
      continue;
    paramSet[index] = sv[1];
  }

  in.close();
}

void simulate(int *paramSet)
{

  const int N = paramSet[0];
  const int L = paramSet[1]; // packet length
  const int R = paramSet[2]; // init timeslot
  const int M = paramSet[3]; // maximum collision
  const int T = paramSet[5]; // total time

  int clocktick = T;

  // output data
  // ratio of clock ticks that were used up for correct communication to the total number of clock ticks
  int utilizedTime = 0;
  int totalColisionNumber = 0;

  //==TODO==
  // comparator on backoff, descending order
  Node *nodes[N] = {};
  for (int i = 0; i < sizeof(nodes); i++)
  {
    nodes[i] = new Node(i);
  }

  while (clocktick > 0)
  {
    cout << "iteration" << (T - clocktick) << endl;

    sort(nodes, nodes + N);
    int endIndexOfSameBackoff = 0;
    int minBackOff = nodes[0]->backoff;
    for (int i = 1; i < sizeof(nodes); i++)
    {
      if (nodes[i]->backoff != minBackOff)
      {
        endIndexOfSameBackoff = i - 1;
        break;
      }
    }

    // all cut backoff by minBackoff
    for (int i = 1; i < sizeof(nodes); i++)
    {
      nodes[i]->backoff = nodes[i]->backoff - minBackOff;
    }

    // check for collision
    for (int i = 0; i < endIndexOfSameBackoff; i++)
    {
      checkForCollision(nodes[i]);
    }

    //

    // clocktick--;
  }
}

int main(int argc, char **argv)
{

  string inputFileName = argv[1];
  // N 25
  // L 20
  // R 8 16 32 64 128 256 512
  // M 6
  // T 50000
  int paramSet[5];
  readFile(paramSet, inputFileName);

  // simulate start
  simulate(paramSet);

  // output file

  return 0;
}
