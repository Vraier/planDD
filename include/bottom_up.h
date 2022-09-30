#pragma once

#include <vector>

#include "logic_primitive.h"

namespace conjoin_order {

// sorts the primitives with the bottom up heuristic
// only sorts inside the range start to end (end is exclusive)
// first: sort by the highest level of the variables (higher (or lower?) levels come first)
// second: sort by the number of affected vars (less affected vars come first)
// then: sort is NOT stable
//TODO: check if the order is actually correct
void sort_bottom_up(std::vector<planning_logic::logic_primitive> &primitives, int start, int end,
                    const std::vector<int> &variable_order);
};