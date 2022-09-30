#include "bottom_up.h"

#include <algorithm>

namespace variable_order {

// returns the highest level that is affected by this primitive
int highest_level(const planning_logic::logic_primitive &primitive, const std::vector<int> &variable_order) {
    int num_variables = variable_order.size();
    int highest_level = num_variables + 1;  // lower values are "higher"
    std::vector<int> affected_variables = primitive.get_affected_variables();
    for (int i = 0; i < affected_variables.size(); i++) {
        int level = variable_order[affected_variables[i]];
        int highest_level = level < highest_level ? level : highest_level;
    }
}

// variable order maps var index to position
void sort_bottom_up(std::vector<planning_logic::logic_primitive> &primitives, int start, int end,
                    const std::vector<int> &variable_order) {
    std::sort(primitives.begin() + start, primitives.begin() + end,
              [&](const planning_logic::logic_primitive &a, const planning_logic::logic_primitive &b) -> bool {
                  if (highest_level(a, variable_order) > highest_level(b, variable_order)) {
                      return true;
                  } else if (highest_level(a, variable_order) == highest_level(b, variable_order)) {
                      if (a.get_affected_variables().size() > b.get_affected_variables().size()) {
                          return true;
                      } else if (a.get_affected_variables().size() == b.get_affected_variables().size()) {
                          return true;
                      } else {
                          return false;
                      }
                  } else {
                      return false;
                  }
              });
}

}  // namespace variable_order