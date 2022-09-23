#pragma once

#include <vector>

#include "logic_primitive.h"

namespace variable_order {

// returns a variable order (pos to idx) given a set of logic primitives.
// the given primitives should not include indizes of variables not included in the initial order
std::vector<int> force_variable_order(std::vector<int> &initial_pos_to_idx,
                                      std::vector<planning_logic::logic_primitive> &primitives);

// returns a vector (pos to clause mapping), the ith entry in the vector indicates, which clause should be conjoined at
// the ith step
std::vector<int> force_clause_order(std::vector<int> &initial_pos_to_idx,
                                    std::vector<planning_logic::logic_primitive> &primitives, int num_variables);

// uses the force algorithm to calculate an ordering for this hypergraph
// position to nodes gives the intial position of the vertices in the ordering
// returns an optiomized position to node mapping
std::vector<int> force_algorithm(std::vector<int> &position_to_node, std::vector<std::vector<int>> &hyper_edges);
};  // namespace variable_order