#ifndef H_DD_BUILDER_VARIABLE_ORDER
#define H_DD_BUILDER_VARIABLE_ORDER

#include <string>
#include <vector>
#include <map>

#include "cnf.h"

namespace variable_order {

// each tag represents a bucket, each tag bucket is divided into subbuckets for each timesetep
// the subbuckets are sorted by the indize of the variable in the planning context (nad then value)
typedef std::map<planning_cnf::variable_tag, std::vector<std::vector<int>>> categorized_variables;


// checks if the string can be used as a build order
bool is_valid_variable_order_string(std::string build_order);

// catgeorizes the variables of a planning cnf into buckets.
// each tag gets a bucket. Each buckets has one subbucket for each timestep
categorized_variables categorize_variables(planning_cnf::cnf &cnf);
// returns a vector V that represents a permutation of the variables of the cnf problem
// the ith entry in V indicates in which layer the ith variable of the cnf should be
// it allows to translate between cnf and bdd world cnf: i <-> bdd: V[i] 
std::vector<int> order_variables(planning_cnf::cnf &cnf, std::string build_order);

};

#endif