#ifndef H_DD_BUILDER_CONJONIN_ORDER
#define H_DD_BUILDER_CONJONIN_ORDER

#include <string>
#include <map>
#include <vector>

#include "cnf.h"

using namespace planning_cnf;

namespace conjoin_order {

// used to interpret the order of clauses from the command line options
std::map<char, clause_tag> char_tag_map = {
    {'i', initial_state}, {'g', goal},  {'r', at_least_var}, {'t', at_most_var}, {'y', at_least_op},
    {'u', at_most_op},    {'m', mutex}, {'p', precondition}, {'e', effect},      {'c', changing_atoms},
};

bool is_valid_build_order_string(std::string build_order);
std::map<clause_tag, std::vector<std::vector<clause>>> categorize_clauses(cnf &cnf);
std::vector<clause> sort_clauses(cnf &cnf, std::string build_order,
                                 std::map<clause_tag, std::vector<std::vector<clause>>> &tagged_clauses);
};

#endif