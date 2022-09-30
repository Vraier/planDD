#include "bottom_up.h"

#include <algorithm>
#include <iostream>

namespace conjoin_order {

// returns the highest level that is affected by this primitive
int highest_level(const planning_logic::logic_primitive &primitive, const std::vector<int> &variable_order) {
    int highest_level = variable_order.size() + 1;  // lower values are "higher"
    std::vector<int> affected_variables = primitive.get_affected_variables();
    for (int i = 0; i < affected_variables.size(); i++) {
        //std::cout << "affected var " << affected_variables[i] << " at pos " << variable_order[affected_variables[i]] << std::endl;

        int level = variable_order[affected_variables[i]];
        highest_level = std::min(level, highest_level);
    }
    return highest_level;
}

// variable order maps var index to position
void sort_bottom_up(std::vector<planning_logic::logic_primitive> &primitives, int start, int end,
                    const std::vector<int> &variable_order) {
    std::sort(primitives.begin() + start, primitives.begin() + end,
              [&](const planning_logic::logic_primitive &a, const planning_logic::logic_primitive &b) {
                  if (highest_level(a, variable_order) == highest_level(b, variable_order)) {
                      //std::cout << "compare lists with same highest level " << highest_level(a, variable_order)
                       //         << ", sa: " << a.get_affected_variables().size()
                       //         << ", sb: " << b.get_affected_variables().size() << std::endl;
                      return a.get_affected_variables().size() >= b.get_affected_variables().size();
                  } else {
                      //std::cout << "highest level a: " << highest_level(a, variable_order)
                      //          << ", b: " << highest_level(b, variable_order) << std::endl;
                      return highest_level(a, variable_order) > highest_level(b, variable_order);
                  }
              });
}

}  // namespace conjoin_order