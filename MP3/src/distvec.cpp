#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

#include "node.h"
// Node{label, distance_vector, neighbors, nexthop} ==> nexthop = map<int(dest), pair<int, int(2nd last hop)>>

using namespace std;

#define DEFAULT_NEXTHOP_INPOSSIBLE_DEST 0;
// here Node.neighbors represent the distance-vector map from source node to all the other nodes

// use ofstream to write documents
ofstream outputFile("output.txt");

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
void init(map<int, Node *> &nodes, string topoFileName)
{
    ifstream in("../" + topoFileName); //be careful to the real file path

    if (!in)
    {
        cout << "Cannot open input file.\n";
        return;
    }

    char str[255];

    //TODO: if using clearTables to do update, then use original neighbors topology to init distancevector and nexthop.
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

        if (!nodes[sv[0]])
        {
            nodes[sv[0]] = new Node(sv[0]);
        }
        if (!nodes[sv[1]])
        {
            nodes[sv[1]] = new Node(sv[1]);
        }
        Node *current_node = nodes[sv[0]];
        (current_node->distance_vector)[sv[1]] = sv[2]; // distance cost of towards that node
        (current_node->next_hop)[sv[1]] = sv[1];        // the next hop node's label
        // neighbor == distance_vector
        (current_node->neighbors)[sv[1]] = sv[2];

        current_node = nodes[sv[1]];
        (current_node->distance_vector)[sv[0]] = sv[2]; // distance cost of towards that node
        (current_node->next_hop)[sv[0]] = sv[0];        // the next hop node's label
        // neighbor == distance_vector
        (current_node->neighbors)[sv[0]] = sv[2];
    }

    //print the nodes map for debugging
    cout << "=======Debug========" << endl;
    cout << "Map nodes:" << endl;
    for (auto &k_v : nodes)
    {
        Node *currentNode = k_v.second;
        for (auto &k_v1 : nodes)
        {
            // to get the  & set, then substract it from itself
            if (k_v.second != k_v1.second)
            {
                if (!k_v.second->distance_vector[k_v1.second->label])
                {
                    k_v.second->distance_vector[k_v1.second->label] = INT32_MAX;
                    // default next-hop for impossible dest
                    k_v.second->next_hop[k_v1.second->label] = DEFAULT_NEXTHOP_INPOSSIBLE_DEST;
                }
            }
        }

        cout << "Node: " << k_v.first << endl;
        for (auto &k_v2 : k_v.second->distance_vector)
        {
            cout << "    distance_vector: " << k_v2.first << ", distance: " << k_v2.second << endl;
        }
        for (auto &k_v2 : k_v.second->next_hop)
        {
            cout << "    dest_label: " << k_v2.first << ", nexthop_label: " << k_v2.second << endl;
        }
        for (auto &k_v2 : k_v.second->neighbors)
        {
            cout << "    neighbors: " << k_v2.first << ", neighbors' original distance: " << k_v2.second << endl;
        }
    }
    in.close();
}

// check all the Node->updated, if all were false, then return true(converged)
bool isConverged(map<int, Node *> &nodes)
{
    for (auto &k_v : nodes)
    {
        if (k_v.second->updated)
        {
            return false;
        }
    }
    return true;
}

// ONce got notified by those updated node, it will call this function to all its neighbors to make it through
bool updateNeighbors(Node *&src, Node *&curr, map<int, Node *> &nodes)
{
    // the message is sent from updated node(src) to current node(curr), to see if it needs to update its distance vector
    bool ret = false;
    for (auto &k_v : curr->distance_vector)
    {
        // if (k_v.first == curr->label)
        // {
        //     continue;
        // }
        int updated_distance;
        int current_distance = k_v.second;
        if (k_v.first == src->label)
        {
            updated_distance = curr->neighbors[src->label];
        }
        else
        {
            int dist_neighbor = curr->distance_vector[src->label];
            int dist_neighbor2dest = src->distance_vector[k_v.first];
            // may overflow integers
            if (dist_neighbor2dest == INT32_MAX)
            {
                continue;
            }
            updated_distance = dist_neighbor + dist_neighbor2dest;
        }

        if (current_distance > updated_distance)
        {
            cout << "update triggered: " << curr->label << "'s neighbor node " << src->label << "  to reduce length from " << current_distance << " to " << updated_distance << endl;
            curr->distance_vector[k_v.first] = updated_distance;
            curr->next_hop[k_v.first] = src->label;
            curr->updated = true;
            ret = true;
        }
        else if (current_distance == updated_distance)
        {
            // check if the routes remain the same
            if ((curr->next_hop)[k_v.first] == src->label)
            {
                continue;
            }
            else
            {
                // TODO, decide the second last hop's label is minimal
                // ret = true;
                // curr->updated = true;
            }
        }
        else
        {
            continue;
        }
    }
    cout << "updateNeighbors()：　node: " << src->label << " sends its update to node: " << curr->label << ".  updated: " << ret << endl;
    return ret;
}

void updateFowardingTables(map<int, Node *> &nodes, bool &converged)
{

    cout << "=============DEBUG=============" << endl;
    cout << "update Forward Tables: " << endl;

    // distributed update the distance vector of each node
    int rnd = 1;
    while (!converged)
    {
        cout << "update forward tables: iteration: " << rnd << endl;
        cout << "============DEBUG===================" << endl;

        bool modified = false;
        for (auto &k_v : nodes)
        {
            if (k_v.second->updated)
            {
                // cout << "prepare for update " << kv.second->label << "'s neighbors. " << endl;
                for (auto &k_v1 : k_v.second->neighbors)
                {
                    if (updateNeighbors(k_v.second, nodes[k_v1.first], nodes))
                    {
                        modified = true;
                    };
                }
                k_v.second->updated = false;
            }
        }
        if (!modified)
        {
            converged = true;
            // ====================DEBUG=====================
            // cout << "iteration " << rnd << " finished and get converged!" << endl;
            // for (auto &k_v : nodes)
            // {
            //     cout << "Node: " << k_v.first << endl;
            //     for (auto &k_v2 : k_v.second->distance_vector)
            //     {
            //         cout << "    distance_vector: " << k_v2.first << ", distance: " << k_v2.second << endl;
            //     }
            //     for (auto &k_v2 : k_v.second->next_hop)
            //     {
            //         cout << "    dest_label: " << k_v2.first << ", nexthop_label: " << k_v2.second << endl;
            //     }
            //     for (auto &k_v2 : k_v.second->neighbors)
            //     {
            //         cout << "    neighbors: " << k_v2.first << ", neighbors' original distance: " << k_v2.second << endl;
            //     }
            //             }
            // exit(0);

            //write to file
            cout << "=======Write Fowarding table to file========" << endl;
            for (auto &k_v : nodes)
            {
                if (outputFile.is_open())
                {
                    for (auto &k_v1 : nodes)
                    {
                        if (k_v1.first == k_v.first)
                        {
                            outputFile << k_v.first << ' ' << k_v.first << ' ' << 0 << endl;
                            continue;
                        }
                        if (k_v.second->distance_vector[k_v1.first] == INT32_MAX)
                            continue;
                        outputFile << k_v1.first << ' ' << k_v.second->next_hop[k_v1.first] << ' ' << k_v.second->distance_vector[k_v1.first] << endl;
                    }
                }
            }
        }
        rnd += 1;
    }
}

void sendAllMsg(string msgFileName, map<int, Node *> &nodes)
{
    cout << "Entering sendAllMsg function" << endl;
    ifstream in("../" + msgFileName); //be careful to the real file path

    if (!in)
    {
        cout << "Cannot open input file.\n";
        return;
    }

    char str[1024];

    while (in)
    {
        in.getline(str, 1024); // delim defaults to '\n'
        if (!in)
            continue; // the last line may be a empty line, skip it
        cout << str << endl;

        //split line
        string temp;

        //get source
        int idx = 0;
        while (str[idx] != ' ')
        {
            temp += str[idx];
            idx++;
        }
        int source = atoi(temp.c_str());

        //get dest
        temp = "";
        idx++;
        while (str[idx] != ' ')
        {
            temp += str[idx];
            idx++;
        }
        int dest = atoi(temp.c_str());

        //get message
        temp = "";
        idx++;
        for (; idx < strlen(str); idx++)
        {
            temp += str[idx];
        }
        string message = temp;

        //from <x> to <y> cost <path_cost> hops <hop1> <hop2> <...> message <message>
        //msg file: "2 1 send this message from 2 to 1"
        //out put: from 2 to 1 cost 6 hops 2 5 4 message here is a message from 2 to 1
        //If the destination is not reach- able,pleasesay
        //output:  from <x> to <y> cost infinite hops unreachable message <message>

        //get cost
        int cost_int = (nodes[source]->distance_vector)[dest];
        string cost = (cost_int == INT32_MAX) ? "infinite" : to_string(cost_int);

        //get path
        string path;
        vector<string> pathVector;
        Node *current_node = nodes[source];
        while (current_node->label != nodes[dest]->label)
        {
            pathVector.push_back(to_string(current_node->next_hop[dest]));
            current_node = nodes[current_node->next_hop[dest]];
        }

        bool firstTime = true;
        for (auto &str : pathVector)
        {
            if (firstTime)
            {
                path += str;
                firstTime = false;
            }
            else
            {
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
    in.close();
}

void changeTopo(int)
{
    // change topo
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }

    string topoFileName = argv[1];
    string msgFileName = argv[2];
    string changeFileName = argv[3];

    cout << topoFileName << ", " << msgFileName << ", " << changeFileName << endl;

    //nexthop and costs
    map<int, Node *> nodes;

    //modified, parse an additional costPairs to document each Node's forwarding tables
    init(nodes, topoFileName);
    bool Converged = false;
    updateFowardingTables(nodes, Converged);
    sendAllMsg(msgFileName, nodes);

    // 3.loop through the change file, change structure and send all messages again
    ifstream in("../" + changeFileName); //be careful to the real file path
    if (!in)
    {
        cout << "Cannot open input file.\n";
    }
    char str[1024];

    while (in)
    {
        in.getline(str, 1024); // delim defaults to '\n'
        if (!in)
            continue; // the last line may be a empty line, skip it, TODO: what does this line do
        cout << str << endl;

        vector<int> sv;
        split(str, sv, ' ');
        if (sv.size() == 0)
            continue;

        int node1 = sv[0];
        int node2 = sv[1];
        int newCost = sv[2];

        cout << "node1 affected: " << node1 << " ,node2 affected: " << node2 << " ,newcost: " << newCost << endl;
        int count_flag = 0;
        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            if (it->first == node1 || it->first == node2)
            {
                count_flag++;
            }
        }
        // validate
        if (count_flag != 2)
        {
            continue;
        }

        if (newCost == -999)
        {
            (nodes[node1]->neighbors).erase(node2);
            // if ((nodes[node2]->neighbors).find(node1) != (nodes[node2]->neighbors).end())
            (nodes[node2]->neighbors).erase(node1);
        }
        else
        {
            (nodes[node1]->neighbors)[node2] = newCost;
            (nodes[node2]->neighbors)[node1] = newCost;
        }

        // clear tables that potentially related to this
        //TODO disttable, nexthop table
        for (auto &node : nodes)
        {
            node.second->updated = true;
            for (auto &k_v : node.second->distance_vector)
            {
                node.second->distance_vector[k_v.first] = INT32_MAX;
                node.second->next_hop[k_v.first] = DEFAULT_NEXTHOP_INPOSSIBLE_DEST;
            }
            for (auto &neighbor : node.second->neighbors)
            {
                node.second->distance_vector[neighbor.first] = neighbor.second;
                node.second->next_hop[neighbor.first] = neighbor.first;
            }
        }

        Converged = false;
        updateFowardingTables(nodes, Converged);
        sendAllMsg(msgFileName, nodes);
    }
    in.close();

    return 0;
}