#pragma once

#include "options.h"
#include "planning_logic_formula.h"
#include "dd_builder_conjoin_order.h"
#include "dd_buildable.h"
#include "bdd_container.h"

namespace dd_builder {
// orders the clauses acording to the given order and conjoins one clause per time
// does no variable ordering
void construct_dd_clause_linear(dd_buildable &dd, std::vector<conjoin_order::tagged_logic_primitiv> &logic_primitives,
                                int dd_index = 0);

// builds a bdd for a single step and then copies it to the opter container for each timestep
void construct_bdd_by_layer(bdd_container &bdd, planning_logic::formula &cnf, option_values &options);
void construct_bdd_by_layer_bidirectional(bdd_container &bdd, planning_logic::formula &cnf, option_values &options);
}  // namespace dd_builder