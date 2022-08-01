#include "graph.h"

#include <unordered_set>
#include <queue>

namespace graph {

undirected_graph::undirected_graph(int num_nodes){
    m_edge_list = std::vector<std::vector<int>>(num_nodes);
}

void undirected_graph::add_edge(int nodeA, int nodeB){
    m_edge_list[nodeA].push_back(nodeB);
    m_edge_list[nodeB].push_back(nodeA);
}

int undirected_graph::get_num_nodes(){
    return m_edge_list.size();
}

std::vector<int> undirected_graph::get_neighbours(int node){
    return m_edge_list[node];
}

// num_colours in neighbourhood, num uncolourd neighbours, node id
typedef std::tuple<int, int, int> node_info;

std::vector<int> approximate_colouring(undirected_graph &graph){

    int n = graph.get_num_nodes();
    std::vector<int> colouring(n, -1);
    std::vector<int> num_uncoloured(n, 0);
    std::vector<std::unordered_set<int>> colours_at_node(n);
    std::priority_queue<node_info> queue;

    // initialize data structures
    for(int i = 0; i < n; i++) {
        int deg_i = graph.get_neighbours(i).size();
        queue.push(std::make_tuple(0, deg_i, i));
    }

    // greedy colour nodes
    while(!queue.empty()){
        node_info curr_best = queue.top(); queue.pop();
        int num_col, node_id, deg_i;
        num_col = std::get<0>(curr_best); 
        node_id = std::get<2>(curr_best); 
        std::vector<int> neighbours = graph.get_neighbours(node_id);
        deg_i = neighbours.size();

        // node is already coloured
        if(colouring[node_id] != -1){
            continue;
        }

        // find minimum unused colour
        int min_colour;
        std::vector<bool> used(deg_i + 1, false);
        for(int i = 0; i < deg_i; i++){
            int neigh_col = colouring[neighbours[i]];
            if(neigh_col != -1){
                used[neigh_col] = true;
            }
        }
        for(int i = 0; i <= deg_i; i++){
            if(!used[i]){
                min_colour = i;
                break;
            }
        }

        // update the node itself
        colouring[node_id] = min_colour;

        // update neighbours
        for(int i = 0; i < deg_i; i++){
            if(colouring[i] != -1) {
                continue;
            }

            int n_id, n_cols, n_uncols;
            n_id = neighbours[i];
            colours_at_node[n_id].insert(min_colour);
            n_cols = colours_at_node[n_id].size();
            num_uncoloured[n_id]--;
            n_uncols = num_uncoloured[n_id];

            queue.push(std::make_tuple(n_cols, n_uncols, n_id));
        }
    }

    return colouring;
}

}