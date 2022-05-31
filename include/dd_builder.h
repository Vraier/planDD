#pragma once

#include <map>
#include <vector>

#include "dd_buildable.h"
#include "planning_logic_formula.h"
#include "options.h"

namespace dd_builder
{    
    void construct_dd_linear_disjoint(dd_buildable &dd, planning_logic::cnf &cnf, option_values &options);
}