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

 int N;
 int L; // packet length
 vector<int> R; // init timeslot
 int M; // maximum collision
 int T; // total time

// // use ofstream to write documents
// ofstream outputFile("output.txt");

bool myCompare(Node* a, Node* b)
{
  return a->backoff < b->backoff;
}

void split(const string &s, vector<string> &sv, const char flag = ' ')
{
  sv.clear();
  istringstream iss(s);
  string temp;
  while (getline(iss, temp, flag))
  {
    sv.push_back(temp);
  }
  return;
}

// second params newly added
void readFile(string inputFile)
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
    vector<string> sv; // node1, node2, distance
    split(str, sv, ' ');
    if (sv.size() == 0)
      continue;

    string type = sv[0];
    if (type == "N") {
        N = stoi(sv[1]);
    } else if (type == "L") {
        L = stoi(sv[1]);
    } else if (type == "R") {
        sv.erase (sv.begin());
        vector<int> r;
        for (auto & str : sv) {
            r.push_back(stoi(str));
        }
        R = r;
    } else if (type == "M") {
        M = stoi(sv[1]);
    } else if (type == "T") {
        T = stoi(sv[1]);
    }
  }

  in.close();
}

void simulate(int *resultSet)
{

  // const int N = paramSet[0];
  // const int L = paramSet[1]; // packet length
  // const int R = paramSet[2]; // init timeslot
  // const int M = paramSet[3]; // maximum collision
  // const int T = paramSet[5]; // total time
  cout << "N: " << N << "L: " << L << "M: " << M  << "T: " << T;
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
    nodes.push_back(new Node(i, R[0]));
    nodes[i]->setRandom(R);
  }

  while (clocktick > 0)
  {
    cout << "iteration: " << (T - clocktick) << endl;

    sort(nodes.begin(), nodes.end(), myCompare);
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
      for (int i = 0; i < endIndexOfSameBackoff + 1; i++)
      {
        nodes[i]->colisionNum++;
        if (nodes[i]->colisionNum >= M)
        {
          nodes[i]->maximalBackoff = R[0];
          nodes[i]->colisionNum = 0;
          nodes[i]->setRandom(R);
        }
        else
        {
          nodes[i]->maximalBackoff *= 2;
          nodes[i]->setRandom(R);
        }

        cout << "node: " << i << "     node's colision num: " << nodes[i]->colisionNum << "     node's maximal Back off:  " << nodes[i]->maximalBackoff << "    node's current random backoff: " << nodes[i]->backoff << endl;
      }
      //TODO:
      totalColisionNumber += (endIndexOfSameBackoff + 1);
    }
    else
    {
      // non collide
      nodes[0]->setRandom(R);

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
  int resultSet[2];
  readFile(inputFileName);

  // simulate start
  simulate(resultSet);

  // output file

  return 0;
}
