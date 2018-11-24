#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <set>
#include <queue>
#include <vector>
#include "node.h"

using namespace std;

class CostPair{
    public:
        int cost;
        int prevNode;
        int dest;
        CostPair(int cost, int prevNode, int dest) {
            this->cost = cost;
            this->prevNode = prevNode;
            this->dest = dest;
        }
};

struct compare {
    bool operator()(CostPair* a, CostPair* b) const {
        if (a->cost != b->cost) return b->cost < a->cost;
        return b->dest < a->dest;
    }
};

ofstream outputFile ("output.txt");
map<int, map<int, CostPair*> > fwdTables;

void split(const string& s,vector<int>& sv,const char flag = ' ') {
    sv.clear();
    istringstream iss(s);
    string temp;

    while (getline(iss, temp, flag)) {
        sv.push_back(stoi(temp));
    }
    return;
}

void init(map<int, Node*> & nodes, string topoFileName) {
    ifstream in(topoFileName); //be careful to the real file path

    if(!in) {
      cout << "Cannot open input file.\n";
      return;
    }

    char str[255];

    while(in) {
      in.getline(str, 255);  // delim defaults to '\n'
      if (!in) continue;
    //   if(in) cout << str << endl;
      cout << str << endl;

      //split line
      vector<int> sv;
      split(str, sv, ' ');
      if (sv.size() == 0) continue;
      //cout << (nodes[sv[0]] == NULL ? "true" : "false") << endl;;
      if (nodes.find(sv[0]) == nodes.end()) {
          nodes[sv[0]] = new Node(sv[0]);
      }

      if (nodes.find(sv[1]) == nodes.end()) {
          nodes[sv[1]] = new Node(sv[1]);
      }

      (nodes[sv[0]]->neighbors)[sv[1]] = sv[2];
      (nodes[sv[1]]->neighbors)[sv[0]] = sv[2];
    }

    //print the nodes map for debugging
    cout << "=======Debug========" << endl;
    cout << "Map nodes:" << endl;
    for (auto & k_v : nodes) {
        cout << "Node: " << k_v.first << endl;
        for (auto & k_v2 : k_v.second-> neighbors) {
            cout << "    neighbor: " << k_v2.first << ", cost: " << k_v2.second << endl;
        }
    }
    in.close();
}

int getNextHops(int source,
                int dest,
                int currNode,
                map<int, Node*> & nodes,
                map<int, int> & nextHops,
                map<int, CostPair*> & costMap) {
    if (nextHops.find(currNode) != nextHops.end()) return nextHops[currNode];
    if (source == dest) {
        nextHops[source] == source;
        return source;
    }
    if (costMap[currNode]->prevNode == source) {
        nextHops[currNode] = currNode;
        return currNode;
    }

    nextHops[currNode] = getNextHops(source, dest, costMap[currNode]->prevNode, nodes, nextHops, costMap);
    return nextHops[currNode];
}

void getFowardingTables(map<int, Node*> & nodes, int source) {
    cout << "entering forwarding table functions" << ", current node label: " << source << endl;

    //for structure change
    map<int, int> nextHops; //5:4 (source -> 5's nexthop is 4)
    map<int, CostPair*> costMap;
    set<int> N;
    int size = nodes.size();

    //initialization of Dijkstra's Algorithm
    map<int, int> srcNeighbors = nodes[source]->neighbors;
    for (auto & k_v : nodes) {
        int currLabel = k_v.first;
        if (currLabel == source) {
            costMap[currLabel] = new CostPair(0, source, currLabel);
        } else if (srcNeighbors.find(currLabel) != srcNeighbors.end()) {
            costMap[currLabel] = new CostPair(srcNeighbors[currLabel], source, currLabel);
        } else {
            costMap[currLabel] = new CostPair(INT_MAX, source, currLabel);
        }
    }
    N.insert(source);
    // cout << "Init Forwarding table for node: " << source  << endl;
    // for (auto & k_v : costMap) {
    //     cout << "Entry Node: " << k_v.first << ", cost: " << k_v.second->cost << endl;
    // }

    //loop
    while (nodes.size() != N.size()) {
        //find the next node to add to set N
        priority_queue<CostPair*, vector<CostPair*>, compare> pq;
        for (auto & k_v : nodes) {
            int currLabel = k_v.first;
            if (N.find(currLabel) != N.end()) continue;
            pq.push(costMap[currLabel]);
        }
        int currNode = pq.top()->dest;
        N.insert(currNode);

        // cout << "current node poped: " << currNode << endl;

        //loop currNode's neighbors and update the distance
        for (auto & k_v : nodes[currNode]->neighbors) {
            int nbLabel = k_v.first;
            if (N.find(nbLabel) != N.end()) continue;
            if (costMap[currNode]->cost == INT_MAX || costMap[currNode]->cost > 1000000) {
                cout << "=====Error=====: Overflow!!!!!!!" << endl;
            }
            int newCost = costMap[currNode]->cost + (nodes[currNode]->neighbors)[nbLabel];
            if (newCost < costMap[nbLabel]->cost) {
                costMap[nbLabel] = new CostPair(newCost, currNode, nbLabel);
            }
        }
    }

    //update the whole Fowarding tables
    fwdTables[source] = costMap;

    //get next hops for every entry
    for (auto & k_v : nodes) {
        getNextHops(source, k_v.first, k_v.first, nodes, nextHops, costMap);
    }

    //print forwarding table
    cout << "=======Debug========" << endl;
    cout << "Forwarding table for node: " << source  << endl;
    for (auto & k_v : costMap) {
        cout << "Entry Node: " << k_v.first << ", nexthop node: " << nextHops[k_v.first] << ", cost: " << k_v.second->cost << endl;
    }

    //write to file
    cout << "=======Write Fowarding table to file========" << endl;
    for (auto & k_v : costMap) {
        if (outputFile.is_open())
        {
          if (k_v.second->cost == INT_MAX) continue;
          outputFile << k_v.first << ' ' << nextHops[k_v.first] << ' ' << k_v.second->cost << endl;
        }
    }

}

void getMsgPath(int target, int currNode, vector<string> & res) {
    if (fwdTables[target][currNode]->prevNode == target) {
        res.push_back(to_string(currNode));
        return;
    }

    res.push_back(to_string(currNode));
    getMsgPath(target, fwdTables[target][currNode]->prevNode, res);
}

void sendAllMsg(string msgFileName) {
    cout << "Entering sendAllMsg function" <<endl;
    cout << "Fwd Tables size: " << fwdTables.size() <<endl;
    ifstream in(msgFileName); //be careful to the real file path

    if(!in) {
      cout << "Cannot open input file.\n";
      return;
    }

    char str[1024];

    while(in) {
      in.getline(str, 1024);  // delim defaults to '\n'
      if (!in) continue;      // the last line may be a empty line, skip it
      cout << str << endl;

      //split line
      string temp;

      //get source
      int idx = 0;
      while (str[idx] != ' ') {
          temp += str[idx];
          idx ++;
      }
      int source = atoi(temp.c_str());

      //get dest
      temp = "";
      idx++;
      while (str[idx] != ' ') {
          temp += str[idx];
          idx ++;
      }
      int dest = atoi(temp.c_str());

      //get message
      temp = "";
      idx++;
      for (;idx < strlen(str);idx++) {
          temp += str[idx];
      }
      string message = temp;

      //􏰃from <x> to <y> cost <path_cost> hops <hop1> <hop2> <...> message <message>􏰄
      //msg file: "2 1 send this message from 2 to 1"
      //out put: from 2 to 1 cost 6 hops 2 5 4 message here is a message from 2 to 1􏰄
      //If the destination is not reach- able,pleasesay􏰃
      //output:  from <x> to <y> cost infinite hops unreachable message <message>

      //get cost
      string cost = (fwdTables[source].find(dest) == fwdTables[source].end()) ? "infinite" : to_string(fwdTables[source][dest]->cost);

      //get path
      string path;
      vector<string> pathVector;
      if ((fwdTables[source].find(dest) == fwdTables[source].end())) {
          pathVector.push_back("unreachable");
      } else {
          getMsgPath(dest, source, pathVector);
      }
      bool firstTime = true;
      for (auto & str : pathVector) {
          if (firstTime) {
              path += str;
              firstTime = false;
          } else {
            path += ' ' + str;
          }
      }

      //write to file
      cout << "=======Debug message to write to file========" << endl;
      cout << "from " << source << " to " << dest << " cost " << cost << " hops " << path << " message " << message << endl;

      //write msg to output file
      cout << "=======Write message to file========" << endl;
      if (outputFile.is_open())
      {
        outputFile << "from " << source << " to " << dest << " cost " << cost << " hops " << path << " message " << message << endl;
      }

    }
}

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }

    string topoFileName = argv[1];
    string msgFileName = argv[2];
    string changeFileName = argv[3];

    cout << topoFileName << ", " << msgFileName << ", " << changeFileName << endl;

    FILE *fpOut;
    fpOut = fopen("output.txt", "w");

    map<int, Node*> nodes;
    //1.initialization
    init(nodes, topoFileName);
    for (auto & k_v : nodes) {
        getFowardingTables(nodes, k_v.first);
    }
    cout << endl;
    cout << endl;

    //2.1st time send message after initialization
    sendAllMsg(msgFileName);
    cout << endl;
    cout << endl;

    //3.loop through the change file, change structure and send all messages again
    ifstream in(changeFileName); //be careful to the real file path
    if(!in) {
      cout << "Cannot open input file.\n";
    }
    char str[1024];

    while(in) {
      in.getline(str, 1024);  // delim defaults to '\n'
      if (!in) continue;      // the last line may be a empty line, skip it
      cout << str << endl;

      vector<int> sv;
      split(str, sv, ' ');
      if (sv.size() == 0) continue;

      int node1 = sv[0];
      int node2 = sv[1];
      int newCost = sv[2];

      //cout << "node1 affected: " << node1 << " ,node2 affected: " << node2 << " ,newcost: " << newCost << endl;
      if (newCost == -999) {
          if ((nodes[node1]->neighbors).find(node2) != (nodes[node1]->neighbors).end()) {
              (nodes[node1]->neighbors).erase(node2);
          }
          if ((nodes[node2]->neighbors).find(node1) != (nodes[node2]->neighbors).end()) {
              (nodes[node2]->neighbors).erase(node1);
          }
      } else {
          (nodes[node1]->neighbors)[node2] = newCost;
          (nodes[node2]->neighbors)[node1] = newCost;
      }

      fwdTables.clear(); //reconstruct the fwdTables for changing structure
      for (auto & k_v : nodes) {
          getFowardingTables(nodes, k_v.first);
      }
      cout << endl;
      cout << endl;

      sendAllMsg(msgFileName);
      cout << endl;
      cout << endl;
    }

    fclose(fpOut);


    return 0;
}



//
// changeStructure() {
// }
