/* File Name: scheduler.cpp
*  Assignment: Assignment 4
*  Submission Date: November 28, 2021
*  Author: Chad Holst
*  UCID: 30105378
*  Course: CPSC 457
*  Instructor: Pavol Federl
*/

// this is the ONLY file you should edit and submit to D2L

/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with edge_index set to the index that caused the deadlock, and dl_procs set
/// to contain with names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with edge_index = -1 and empty dl_procs[].
///

#include "deadlock_detector.h"
#include "common.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

Result result;
vector<string> arrows;
vector<int> outCopy;
vector<int> zeros;
int edgeCount = -1;
Word2Int convert;
vector<string> names;
 // Node IDs (P and R can have same) adj_list["node"] contains a list of all nodes with edges pointing towards "node"
vector<int> out_counts; // outgoing degree, if 0, then remove from graph. This needs each process and resource degrees
// string is Node name/id is string & int is outgoing degree

bool checkNodes(int uniqueValue, string name) // check if unique node name has been added to adjacency list
{
    bool check = false;
    if(names.at(uniqueValue).compare(name) == 0)
    {
        check = true;
    }
    return check;
}

Result detect_deadlock(const vector<string>& edges) // create graphs & dynamically check deadlocks
{
    if(edges.size() > 30000 || edges.size() < 0) // ensure assumptions within range
    {
        cerr << "Error: must have edges within range" << endl;
    }

    string tempString = "";
    vector<string> tempVect;
    vector<vector<int>> adj_list;
    names.resize(edges.size()*2); // pre allocate maximum possible unique node names (with prefix) 
    int countNodes = 0;
    for(int i = 0; i < (int)edges.size(); i++) // add P, arrow, and Q. need to add P and Q to hash map as Nodes
    {
        tempString = edges.at(i);
        tempVect = split(tempString);

        string Pname = "P_" + tempVect.at(0);
        int proc = convert.get(Pname); // processes
        
        arrows.push_back(tempVect.at(1)); // arrows

        string Rname = "R_" + tempVect.at(2);
        int res = convert.get(Rname); // resources

        names[proc] = Pname;
        names[res] = Rname;

        if(checkNodes(proc,Pname)) countNodes++; // check if unique node name exists for sizing vector
        if(checkNodes(res,Rname)) countNodes++;

        adj_list.resize(countNodes); // resize vectors to existing amount of unique node names (with prefix)
        out_counts.resize(countNodes);
        
        if((checkNodes(proc, Pname)) && (arrows.at(i).compare("<-") == 0)) // append incoming
        {
           adj_list[proc].push_back(res);
        }
        
        if((checkNodes(res, Rname)) && (arrows.at(i).compare("->") == 0))
        {
            adj_list[res].push_back(proc); 
        }

        if((checkNodes(proc, Pname)) && (arrows.at(i).compare("->") == 0)) // increment outgoing
        {
            out_counts[proc]++;
        }
        if((checkNodes(res, Rname)) && (arrows.at(i).compare("<-") == 0)) 
        {
            out_counts[res]++;
        }
     
        outCopy = out_counts; // use overloaded operator = to copy vector
        for(size_t j = 0; j < outCopy.size(); j++) // copy out_counts to modify & find all nodes with zero outgoing degree
        {
            if(outCopy.at(j) == 0) // find zeros
            {
                zeros.push_back(j); // add node name to zeros vector if out degree of 0
            }
        }

        while(!zeros.empty()) // keep removing nodes with outgoing degree of 0 for topological sort
        {
            auto node1 = zeros.back(); // most efficient access is back of vector
            zeros.pop_back();
    
            for(auto node2: adj_list.at(node1))
            {
                outCopy[node2]--;
                if(outCopy.at(node2) == 0)
                {
                    zeros.push_back(node2);
                }
            }
        }

        for(int i = 0; i < (int)outCopy.size(); i++) // check cycle for out degree > 0
        {
            int outCount = outCopy.at(i);
            if(outCount <= 0) continue;
            string node = names.at(i);
            if((outCount > 0) && (node.substr(0,2).compare("P_") == 0))
            {
                node.erase(0,2);
                result.dl_procs.push_back(node);
            }
        }

        if(result.dl_procs.empty() == false) break; 
        else result.edge_index++;    
    }

    if(result.dl_procs.empty()) // no cycle if no node names were appended
    {
        result.edge_index = -1;
    }
    
    return result;
}