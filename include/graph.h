#pragma once

#include <vector>
#include <string>

namespace graph
{

class undirected_graph {
   private:
    std::vector<std::vector<int>> m_edge_list;

   public:
    undirected_graph(int num_nodes);

    void add_edge(int nodeA, int nodeB);

    int get_num_nodes();
    std::vector<int> get_neighbours(int node);

    void write_to_file(std::string filepath);
};

// uses greedy colouring to colour graph
// always colours the node with the most different colours in its neighbourhood
std::vector<int> approximate_colouring(undirected_graph &graph);
}