#pragma once

#include <string>
#include <vector>
#include <map>

#include "planning_logic_formula.h"
#include "options.h"

namespace variable_order {

// each tag represents a bucket, each tag bucket is divided into subbuckets for each timesetep
// the subbuckets are sorted by the indize of the variable in the planning context (and then value)
typedef std::map<planning_logic::variable_tag, std::vector<std::vector<int>>> categorized_variables;


// checks if the string can be used as a build order
bool is_valid_variable_order_string(std::string build_order);

// catgeorizes the variables of a planning cnf into buckets.
// each tag gets a bucket. Each buckets has one subbucket for each timestep
categorized_variables categorize_variables(planning_logic::formula &cnf);
// Moves all the variables to the front that are in a clause with the given front_tag
// current_order[i] = what variable is at layer i?
std::vector<int> put_variables_of_tag_first(planning_logic::formula &cnf, std::vector<int> &current_order, planning_logic::primitive_tag front_tag);
// returns a vector V that represents a permutation of the variables of the cnf problem
// The i-th entry of the permutation array contains the index of the variable that should be brought to the i-th level
std::vector<int> order_variables(planning_logic::formula &cnf, option_values &options);

};