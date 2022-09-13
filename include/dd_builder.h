#pragma once

#include "encoder_abstract.h"
#include "options.h"
#include "planning_logic_formula.h"
#include "dd_builder_conjoin_order.h"
#include "dd_buildable.h"
#include "bdd_container.h"
#include "logic_primitive.h"

namespace dd_builder {

// modifies the container
// chooses the correct construction algorithm according to options
void construct_dd(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);

// refactored method to construct dd when the amount of timesteps is known
// allows for finegrained control of variable and clause ordering (with force or bottom up heuristic)
void construct_dd_with_timesteps(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);

// conjoins one logic primitive at the time to the bdd
void construct_dd_linear(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);
// incremental, when the timesteps are not known
void construct_dd_without_timesteps(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);
// builds a bdd for a single step and then copies it to the opter container for each timestep
void construct_dd_by_layer_unidirectional(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);
void construct_dd_by_layer_bidirectional(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);
void construct_dd_by_layer_exponentially(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options);

// helper method
// conjoins all the logic primitives to the given bdd.
// can be turned silent
void conjoin_primitives_linear(dd_buildable &dd, std::vector<planning_logic::logic_primitive> &logic_primitives,
                                int dd_index = 0, bool silent = false);
}  // namespace dd_builder