#include "force.h"

#include <iostream>
#include <algorithm>  // std::sort
#include <cmath>      // std::log2

#include "logging.h"


namespace variable_order {

std::vector<int> force_variable_order(std::vector<int> &initial_pos_to_idx,
                                      std::vector<planning_logic::logic_primitive> &primitives) {
    
    std::vector<std::vector<int>> hyper_edges;
    for (int p = 0; p < primitives.size(); p++) {
        hyper_edges.push_back(primitives[p].get_affected_variables());
    }
    return force_algorithm(initial_pos_to_idx, hyper_edges);
}

std::vector<int> force_clause_order(std::vector<int> &initial_pos_to_idx,
                                      std::vector<planning_logic::logic_primitive> &primitives, int num_variables){
                            
    std::vector<std::vector<int>> hyper_edges(num_variables);
    for(int p = 0; p < primitives.size(); p++) {
        std::vector<int> affected_vars = primitives[p].get_affected_variables();
        for(int v = 0; v < affected_vars.size(); v++){
            int affected_var = affected_vars[v];
            hyper_edges[affected_var].push_back(p);
        }
    }

    return force_algorithm(initial_pos_to_idx, hyper_edges);
}

// calculates the total span of all hyper edges
// the span of a hyper edges is the difference between its lowest and highest node
int calculate_total_span(std::vector<int> &node_to_pos, std::vector<std::vector<int>> &hyper_edges) {
    int sum = 0;
    for (int h = 0; h < hyper_edges.size(); h++) {
        if(hyper_edges[h].size() == 0){
            continue;
        }

        // calculate the span of the hyper edge
        int left_pos = node_to_pos.size();
        int right_pos = -1;
        for (int v = 0; v < hyper_edges[h].size(); v++) {
            int node = hyper_edges[h][v];
            int pos = node_to_pos[node];

            left_pos = pos < left_pos ? pos : left_pos;
            right_pos = pos > right_pos ? pos : right_pos;
        }

        sum += (right_pos - left_pos);
    }
    return sum;
}

std::vector<int> force_algorithm(std::vector<int> &position_to_node, std::vector<std::vector<int>> &hyper_edges) {
    int num_vertices = position_to_node.size();
    int num_hyperedges = hyper_edges.size();

    const int ITERATION_MULTIPLYER = 3;
    int max_iterations = ITERATION_MULTIPLYER * std::log2(num_vertices);

    std::vector<int> curr_position_to_node = position_to_node;
    std::vector<int> curr_node_to_position(num_vertices);

    for (int pos = 0; pos < num_vertices; pos++) {
        curr_node_to_position[curr_position_to_node[pos]] = pos;
    }

    int iteration = 0;
    int initial_span = calculate_total_span(curr_node_to_position, hyper_edges);
    int current_span = initial_span;
    int last_span = current_span + 1;
    while (iteration < max_iterations && last_span - current_span > 0) {
        // calculate center of gravity for every hyper edge
        std::vector<double> center_of_gravity(num_hyperedges);
        for (int h = 0; h < num_hyperedges; h++) {
            double pos_sum = 0;
            for (int v = 0; v < hyper_edges[h].size(); v++) {
                pos_sum += curr_node_to_position[hyper_edges[h][v]];
            }
            center_of_gravity[h] = pos_sum / hyper_edges[h].size();
        }

        // calculate new positioning for nodes
        std::vector<double> pos_sum(num_vertices);
        std::vector<int> num_edges(num_vertices);
        for (int h = 0; h < num_hyperedges; h++) {
            for (int v = 0; v < hyper_edges[h].size(); v++) {
                int node = hyper_edges[h][v];
                int cog = center_of_gravity[h];

                pos_sum[node] += cog;
                num_edges[node] += 1;
            }
        }

        // sort nodes according to new position
        std::vector<std::pair<double, int>> pos_node_pairs;
        for (int v = 0; v < num_vertices; v++) {
            if (num_edges[v] != 0){
                pos_node_pairs.push_back(std::make_pair(pos_sum[v] / num_edges[v], v));
            } else {
                pos_node_pairs.push_back(std::make_pair(0.0f, v));
            }
        }
        
        std::sort(pos_node_pairs.begin(), pos_node_pairs.end());

        // calculate the node to position mappings again
        for (int v = 0; v < num_vertices; v++) {
            curr_position_to_node[v] = pos_node_pairs[v].second;
            curr_node_to_position[curr_position_to_node[v]] = v;
        }

        // update counters
        iteration++;
        last_span = current_span;
        current_span = calculate_total_span(curr_node_to_position, hyper_edges);
    }

    LOG_MESSAGE(log_level::info) << "Force did " << iteration << " iterations, went from " << initial_span/num_hyperedges << " to " << current_span/num_hyperedges;

    return curr_position_to_node;
}

};  // namespace variable_order