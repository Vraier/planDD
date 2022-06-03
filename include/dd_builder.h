#pragma once

#include "dd_buildable.h"
#include "planning_logic_formula.h"
#include "options.h"
#include "bdd_container.h"

namespace dd_builder
{    
    // orders the clauses acording to the given order and conjoins one clause per time
    // does no variable ordering
    void construct_dd_clause_linear(dd_buildable &dd, planning_logic::formula &cnf, option_values &options);

    // builds a bdd for a single step and then copies it to the opter container for each timestep
    void construct_bdd_by_layer(bdd_container &main_bdd, bdd_container &single_step_bdd, planning_logic::formula &cnf, option_values &options);
}