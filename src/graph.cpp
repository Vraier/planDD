#include "graph.h"

#include "logging.h"

#include <unordered_set>
#include <queue>
#include <iostream>
#include <fstream>

namespace graph {

undirected_graph::undirected_graph(int num_nodes) { m_edge_list = std::vector<std::vector<int>>(num_nodes); }

void undirected_graph::add_edge(int nodeA, int nodeB) {
    m_edge_list[nodeA].push_back(nodeB);
    m_edge_list[nodeB].push_back(nodeA);
}

int undirected_graph::get_num_nodes() { return m_edge_list.size(); }

bool undirected_graph::are_neighbours(int nodeA, int nodeB) {
    for (int x : get_neighbours(nodeA)) {
        if (x == nodeB) {
            return true;
        }
    }
    return false;
}

std::vector<int> undirected_graph::get_neighbours(int node) { return m_edge_list[node]; }

undirected_graph undirected_graph::construct_complement() {
    int num_nodes = get_num_nodes();
    undirected_graph result(get_num_nodes());

    for (int i = 0; i < num_nodes; i++) {
        for (int j = i + 1; j < num_nodes; j++) {
            if (!are_neighbours(i, j)) {
                result.add_edge(i, j);
            }
        }
    }

    return result;
}

void write_to_file(std::string filepath, undirected_graph &graph) {
    std::ofstream dot_file(filepath);
    if (!dot_file.is_open()) {
        LOG_MESSAGE(log_level::error) << "Unable to open file " << filepath;
    }
    dot_file << "graph {\n";
    dot_file << "\toverlap=scale;\n";
    dot_file << "\tsplines=true;\n";

    auto edges = graph.m_edge_list;

    for (int i = 0; i < edges.size(); i++) {
        for (int j = 0; j < edges[i].size(); j++) {
            int neighbour = edges[i][j];

            if (neighbour > i) {
                dot_file << "\t" << i << " -- " << neighbour << ";\n";
            }
        }
    }

    dot_file << "}";
    dot_file.close();
}

void write_to_file_with_colouring(std::string filepath, undirected_graph &graph, std::vector<int> &colouring,
                                  bool cluster) {
    int max_col = 0;
    for (int i = 0; i < colouring.size(); i++) {
        max_col = colouring[i] > max_col ? colouring[i] : max_col;
    }

    if (max_col > 8) {
        write_to_file(filepath, graph);
        return;
    }

    std::ofstream dot_file(filepath);
    if (!dot_file.is_open()) {
        LOG_MESSAGE(log_level::error) << "Unable to open file " << filepath;
    }

    dot_file << "graph {\n";
    dot_file << "\toverlap=scale;\n";
    dot_file << "\tsplines=true;\n";

    dot_file << "\tnode [colorscheme=dark28, style=filled] # Apply colorscheme to all nodes\n";
    if (!cluster) {
        for (int i = 0; i < colouring.size(); i++) {
            dot_file << "\t" << i << " [color=" << colouring[i] + 1 << "]\n";
        }
    } else {
        for (int i = 0; i <= max_col; i++) {
            dot_file << "\tsubgraph cluster_" << i << " {\n";
            for (int j = 0; j < colouring.size(); j++) {
                if (colouring[j] == i) {
                    dot_file << "\t\t" << j << " [color=" << i + 1 << "]\n";
                }
            }
            dot_file << "\t}\n";
        }
    }

    auto edges = graph.m_edge_list;
    for (int i = 0; i < edges.size(); i++) {
        for (int j = 0; j < edges[i].size(); j++) {
            int neighbour = edges[i][j];

            if (neighbour > i) {
                dot_file << "\t" << i << " -- " << neighbour << ";\n";
            }
        }
    }

    dot_file << "}";
    dot_file.close();
}

// num_colours in neighbourhood, num uncolourd neighbours, node id
typedef std::tuple<int, int, int> node_info;

std::vector<int> approximate_colouring(undirected_graph &graph) {
    int n = graph.get_num_nodes();
    std::vector<int> colouring(n, -1);
    std::vector<int> num_uncoloured(n, 0);
    std::vector<std::unordered_set<int>> colours_at_node(n);
    std::priority_queue<node_info> queue;

    // initialize data structures
    for (int i = 0; i < n; i++) {
        int deg_i = graph.get_neighbours(i).size();
        queue.push(std::make_tuple(0, deg_i, i));
    }

    // greedy colour nodes
    while (!queue.empty()) {
        node_info curr_best = queue.top();
        queue.pop();
        int node_id, deg_i;
        node_id = std::get<2>(curr_best);
        std::vector<int> neighbours = graph.get_neighbours(node_id);
        deg_i = neighbours.size();

        // node is already coloured
        if (colouring[node_id] != -1) {
            continue;
        }

        // find minimum unused colour
        int min_colour;
        std::vector<bool> used(deg_i + 1, false);
        for (int i = 0; i < deg_i; i++) {
            int neigh_col = colouring[neighbours[i]];
            if (neigh_col != -1) {
                used[neigh_col] = true;
            }
        }
        for (int i = 0; i <= deg_i; i++) {
            if (!used[i]) {
                min_colour = i;
                break;
            }
        }

        // update the node itself
        colouring[node_id] = min_colour;

        // update neighbours
        for (int i = 0; i < deg_i; i++) {
            if (colouring[i] != -1) {
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

}  // namespace graph