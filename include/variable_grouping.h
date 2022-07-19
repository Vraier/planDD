#pragma once

#include "cnf_encoder.h"
#include "bdd_container.h"
#include "options.h"

/*
 * The cnf encoder adds a lot of variables to its symbol map.
 * These variables also have to be created for the dd managers.
 * This class creates the variables for and groups them for the dd manager.
 */
namespace variable_grouping {

void create_all_variables(cnf_encoder &encoder, bdd_container &container, option_values &options);
}  // namespace variable_grouping