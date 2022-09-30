#pragma once

#include <map>
#include <string>
#include <vector>

#include "encoder_basic.h"
#include "logic_primitive.h"
#include "options.h"

namespace conjoin_order {

extern std::map<char, planning_logic::primitive_tag> char_tag_map;

// checks if the string can be used as a build order
bool is_valid_conjoin_order_string(std::string &build_order);
bool is_valid_layer_order_string(std::string &layer_order);

// uses the categorized clauses to order them in one single vector according to a build order
std::vector<planning_logic::logic_primitive> order_all_clauses(encoder::encoder_abstract &encoder,
                                                               option_values &options);

// helper methods to order logic primitives. The correct ordering method will be used by order_all_clauses
std::vector<std::tuple<planning_logic::logic_primitive, int, int>> create_custom_force_clause_order_mapping(
    encoder::encoder_abstract &encoder, option_values &options);
std::vector<std::tuple<planning_logic::logic_primitive, int>> create_force_clause_order_mapping(
    encoder::encoder_abstract &encoder, option_values &options);
std::vector<std::tuple<planning_logic::logic_primitive, int>> create_custom_clause_order_mapping(
    encoder::encoder_abstract &encoder, option_values &options);
std::vector<std::tuple<planning_logic::logic_primitive, int>> create_bottom_up_clause_order_mapping(
    encoder::encoder_abstract &encoder, option_values &options);

// same as order clauses but only orders the precondition, effect and frame clauses for timeset 0
std::vector<planning_logic::logic_primitive> order_clauses_for_layer(encoder::encoder_abstract &encoder,
                                                                     std::string &order_string, int layer);

// same as order clauses but only orders the initial state, goal and mutex
std::vector<planning_logic::logic_primitive> order_clauses_for_foundation(encoder::encoder_abstract &encoder,
                                                                          std::string &order_string, int timesteps);

// helper methods:
// find the primitives of all timesteps for a given primitive type. ordered by timesteps ascending
std::vector<planning_logic::logic_primitive> collect_primitives_for_all_timesteps(
    encoder::encoder_abstract &encoder, planning_logic::primitive_tag primitive_type, int timesteps);
// find the primitives of a specifiv timestep
std::vector<planning_logic::logic_primitive> collect_primitives_for_single_timestep(
    encoder::encoder_abstract &encoder, planning_logic::primitive_tag primitive_type, int timestep);
};  // namespace conjoin_order