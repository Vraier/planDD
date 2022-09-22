#pragma once

#include <vector>

#include "logic_primitive.h"

namespace visualize {

    // produces an svg file the visualiyes the variable order
    // clauses are placed in the middle of their spans
    void visualize_var_order(std::vector<int> &var_order, std::vector<planning_logic::logic_primitive> &primitives);
}