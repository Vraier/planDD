#pragma once

#include <string>
#include <map>
#include <vector>

#include "planning_logic_formula.h"
#include "options.h"

namespace conjoin_order {

typedef std::pair<planning_logic::logic_primitive, planning_logic::logic_primitive_type> tagged_logic_primitiv;

// each tag represents a buckets, each tag buckets is divided into subbuckets for each timesetep
// the subbuckets have an arbitrary order and contain the actual cnf clauses
typedef std::map<planning_logic::clause_tag, std::vector<std::vector<planning_logic::clause>>> categorized_clauses;
typedef std::map<planning_logic::eo_constraint_tag, std::vector<std::vector<planning_logic::eo_constraint>>> categorized_constraints;

// checks if the string can be used as a build order
bool is_valid_conjoin_order_string(std::string build_order);

// catgeorizes the clauses of a planning cnf into buckets.
// each tag gets a bucket. Each buckets is hase one subbucket for each timestep
categorized_clauses categorize_clauses(planning_logic::formula &cnf);
// same for the planning constraints
categorized_constraints categorize_constraints(planning_logic::formula &cnf);

// uses the categorized clauses to order them in one single vector according to a build order
std::vector<tagged_logic_primitiv> order_clauses(planning_logic::formula &cnf, option_values &options);
};