#pragma once

#include <string>
#include <map>
#include <vector>

#include "planning_logic_formula.h"
#include "options.h"

namespace conjoin_order {

typedef std::pair<planning_logic::logic_primitive, planning_logic::logic_primitive_type> tagged_logic_primitiv;

// checks if the string can be used as a build order
bool is_valid_conjoin_order_string(std::string build_order);

// catgeorizes the clauses of a planning cnf into buckets.
// each tag gets a bucket. Each buckets is hase one subbucket for each timestep
void print_info_about_number_of_logic_primitives(planning_logic::formula &cnf);

// uses the categorized clauses to order them in one single vector according to a build order
std::vector<tagged_logic_primitiv> order_clauses(planning_logic::formula &cnf, option_values &options);

// same as order clauses but only orders the precondition, effect and frame clauses for timeset 0
std::vector<tagged_logic_primitiv> order_clauses_for_single_timestep(planning_logic::formula &cnf, option_values &options);
};