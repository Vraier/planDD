#pragma once

#include <map>
#include <string>
#include <vector>

#include "encoder_basic.h"
#include "options.h"
#include "planning_logic_formula.h"

namespace variable_order {

// each tag represents a bucket, each tag bucket is divided into subbuckets for each timesetep
// the subbuckets are sorted by the indize of the variable in the planning context (and then value)
typedef std::map<planning_logic::variable_tag, std::vector<std::vector<int>>> categorized_variables;

// checks if the string can be used as a build order
bool is_valid_variable_order_string(std::string build_order);

// catgeorizes the variables of a planning cnf into buckets.
// each tag gets a bucket. Each buckets has one subbucket for each timestep
categorized_variables categorize_variables(planning_logic::plan_to_cnf_map &symbol_map, int timesteps);
// Moves all the variables to the front that are in a clause with the given front_tag
// current_order[i] = what variable is at layer i?
std::vector<int> put_variables_of_tag_first(encoder::encoder_abstract *encoder, std::vector<int> &current_order,
                                            planning_logic::primitive_tag front_tag, int timesteps);

// returns a vector V that represents a permutation of the variables of the cnf problem
// The i-th entry of the permutation array contains the index of the variable that should be brought to the i-th level
// chooses the correct ordering method and applies post processing
std::vector<int> order_variables(encoder::encoder_abstract &encoder, option_values &options);

// helper methods to order variables. The correct ordering method will be used by order_variables
// the tuples have the var index at the first positions.
// the tuples are sorted in the correct order
// the second and first entry have information about the toal/partial orer of the variables
std::vector<std::tuple<int, int, int>> create_custom_force_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                    option_values &options);
std::vector<std::tuple<int, int>> create_force_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                 option_values &options);
std::vector<std::tuple<int, int>> create_custom_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                  option_values &options);

};  // namespace variable_order