#include "bottom_up.h"


namespace variable_order {

// returns the highest leves that is affected by this primitive
int highest_level(planning_logic::logic_primitive &primitive, std::vector<int> &variable_order) {
    int num_variables = variable_order.size();
    int highest_level = num_variables + 1;
    std::vector<int> affected_variables()
    for(int i = 0; i <)
}

// variable order maps var index to position
void sort_bottom_up(std::vector<planning_logic::logic_primitive> &primitives, int start, int end, std::vector<int> &variable_order){

}

}