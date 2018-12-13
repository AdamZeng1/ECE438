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

bool compare(Node const &a, Node const &b)
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

void simulate(int *paramSet, int *resultSet)
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

  vector<Node *> nodes;

  for (int i = 0; i < N; i++)
  {
    nodes.push_back(new Node(i, R));
    nodes[i]->setRandom();
  }

  while (clocktick > 0)
  {
    cout << "iteration: " << (T - clocktick) << endl;

    sort(nodes.begin(), nodes.end(), compare);
    int endIndexOfSameBackoff = 0;
    int minBackOff = nodes[0]->backoff;

    // find the collision nodes
    for (int i = 1; i < nodes.size(); i++)
    {
      if (nodes[i]->backoff != minBackOff)
      {
        endIndexOfSameBackoff = i - 1;
        break;
      }
    }

    // all cut backoff by minBackoff
    for (int i = 0; i < nodes.size(); i++)
    {
      nodes[i]->backoff = nodes[i]->backoff - minBackOff;
    }

    // if collision
    if (endIndexOfSameBackoff > 0)
    {
      // collision number ++
      for (int i = 0; i < endIndexOfSameBackoff; i++)
      {
        nodes[i]->colisionNum++;
        if (nodes[i]->colisionNum >= M)
        {
          nodes[i]->maximalBackoff = R;
          nodes[i]->colisionNum = 0;
          nodes[i]->setRandom();
        }
        else
        {
          nodes[i]->maximalBackoff *= 2;
          nodes[i]->setRandom();
        }
      }
      //TODO:
      totalColisionNumber += (endIndexOfSameBackoff + 1);
    }
    else
    {
      // non collide
      nodes[0]->setRandom();

      // file transmission
      utilizedTime += L;
      clocktick -= L;
    }

    //
    clocktick -= minBackOff;
  }

  resultSet[0] = utilizedTime;
  resultSet[1] = clocktick;
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
  int resultSet[2];
  readFile(paramSet, inputFileName);

  // simulate start
  simulate(paramSet, resultSet);

  // output file

  return 0;
}
