#pragma once

#include "options.h"
#include "planning_logic_formula.h"
#include "dd_builder_conjoin_order.h"
#include "dd_buildable.h"
#include "bdd_container.h"
#include "logic_primitive.h"

namespace dd_builder {

// chooses the correct construction algorithm according to options
dd_buildable construct_dd(cnf_encoder &encoder, option_values &options);

// conjoins one logic primitive at the time to the bdd
dd_buildable construct_dd_linear(cnf_encoder &encoder, option_values &options);
// incremental, when the timesteps are not known
bdd_container construct_bdd_without_timesteps(cnf_encoder &encoder, option_values &options);
// builds a bdd for a single step and then copies it to the opter container for each timestep
bdd_container construct_bdd_by_layer_unidirectional(cnf_encoder &encoder, option_values &options);
bdd_container construct_bdd_by_layer_bidirectional(cnf_encoder &encoder, option_values &options);
bdd_container construct_dd_by_layer_exponentially(cnf_encoder &encoder, option_values &options);

// helper method
// conjoins all the logic primitives to the given bdd.
// can be turned silent
void construct_dd_clause_linear(dd_buildable &dd, std::vector<planning_logic::logic_primitive> &logic_primitives,
                                int dd_index = 0, bool silent = false);
}  // namespace dd_builder