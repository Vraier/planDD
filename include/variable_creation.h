#pragma once

#include "encoder_basic.h"
#include "bdd_container.h"
#include "options.h"

/*
 * The cnf encoder adds a lot of variables to its symbol map.
 * These variables also have to be created for the bdd managers.
 * This class creates the variables and groups them for the bdd manager.
 */
namespace variable_creation {

// creates the variables for all timesteps upto t
// possible also groups variables (depending on options struct)
void create_variables_for_first_t_steps(int t, encoder::encoder_abstract &encoder, dd_buildable &container, option_values &options);

// only creates the variables for the given timestep
// in timestep 0 only state variables are created
// in all other timesteps also the action variables (for t-1) are created
void create_variables_for_timestep_t(int t, encoder::encoder_abstract &encoder, dd_buildable &container, option_values &options);
}  // namespace variable_grouping