#pragma once

#include "options.h"
#include "planning_logic_formula.h"
#include "dd_builder_conjoin_order.h"
#include "dd_buildable.h"
#include "bdd_container.h"
#include "logic_primitive.h"

namespace dd_builder {
// TODO remove symbol map from beeing necessary
// orders the clauses acording to the given order and conjoins one clause per time
// does no variable ordering
void construct_dd_clause_linear(dd_buildable &dd, std::vector<planning_logic::logic_primitive> &logic_primitives,
                                int dd_index = 0, bool silent = false);

// builds a bdd for a single step and then copies it to the opter container for each timestep
void construct_bdd_by_layer_unidirectional(bdd_container &bdd, cnf_encoder &encoder,
                                           planning_logic::plan_to_cnf_map &symbol_map, option_values &options);
void construct_bdd_by_layer_bidirectional(bdd_container &bdd, cnf_encoder &encoder,
                                          planning_logic::plan_to_cnf_map &symbol_map, option_values &options);
void construct_dd_by_layer_exponentially(bdd_container &bdd, cnf_encoder &encoder,
                                         planning_logic::plan_to_cnf_map &symbol_map, option_values &options);
void construct_bdd_without_timesteps(bdd_container &bdd, cnf_encoder &encoder,
                                     planning_logic::plan_to_cnf_map &symbol_map, option_values &options);
}  // namespace dd_builder