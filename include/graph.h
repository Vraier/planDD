#pragma once

#include <vector>
#include <string>

namespace graph
{

class undirected_graph {
   public:
    std::vector<std::vector<int>> m_edge_list;
    
    undirected_graph(int num_nodes);

    void add_edge(int nodeA, int nodeB);

    int get_num_nodes();
    bool are_neighbours(int nodeA, int nodeB);
    std::vector<int> get_neighbours(int node);

    undirected_graph construct_complement();
};

void write_to_file(std::string filepath, undirected_graph &graph);
// the colouring should use at most 12 colours
void write_to_file_with_colouring(std::string filepath, undirected_graph &graph, std::vector<int> &colouring, bool cluster);

// uses greedy colouring to colour graph
// always colours the node with the most different colours in its neighbourhood
std::vector<int> approximate_colouring(undirected_graph &graph);
}