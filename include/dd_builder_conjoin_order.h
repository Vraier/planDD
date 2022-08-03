#pragma once

#include <string>
#include <map>
#include <vector>

#include "cnf_encoder.h"
#include "logic_primitive.h"
#include "options.h"

namespace conjoin_order {

extern std::map<char, planning_logic::primitive_tag> char_tag_map;

// checks if the string can be used as a build order
bool is_valid_conjoin_order_string(std::string &build_order);
bool is_valid_layer_order_string(std::string &layer_order);

// uses the categorized clauses to order them in one single vector according to a build order
std::vector<planning_logic::logic_primitive> order_all_clauses(encoder::cnf_encoder &encoder, option_values &options);

// same as order clauses but only orders the precondition, effect and frame clauses for timeset 0
std::vector<planning_logic::logic_primitive> order_clauses_for_layer(encoder::cnf_encoder &encoder, std::string &order_string,
                                                                     int layer);

// same as order clauses but only orders the initial state, goal and mutex
std::vector<planning_logic::logic_primitive> order_clauses_for_foundation(encoder::cnf_encoder &encoder,
                                                                          std::string &order_string, int timesteps);

// helper methods:
// find the primitives of all timesteps for a given primitive type. ordered by timesteps ascending
std::vector<planning_logic::logic_primitive> collect_primitives_for_all_timesteps(
    encoder::cnf_encoder &encoder, planning_logic::primitive_tag primitive_type, int timesteps);
// find the primitives of a specifiv timestep
std::vector<planning_logic::logic_primitive> collect_primitives_for_single_timestep(
    encoder::cnf_encoder &encoder, planning_logic::primitive_tag primitive_type, int timestep);
};  // namespace conjoin_order