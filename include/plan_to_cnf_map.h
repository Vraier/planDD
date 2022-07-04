#pragma once

#include <map>
#include <vector>

namespace planning_logic {

enum variable_tag {
    variable_plan_var,
    variable_plan_op,
    variable_h_amost_variable,
    variable_h_amost_operator,
    variable_h_amost_mutex,
    variable_none,
};

// variable tag, timestep, index of the variable (and value if it exist, 0 else)
typedef std::tuple<variable_tag, int, int, int> tagged_variable;

class plan_to_cnf_map {
   private:
    // maps information about planning variable to variable index of cnf formula
    // are used by the variable ordering algorithm
    std::map<tagged_variable, int> m_variable_map;
    std::map<int, tagged_variable> m_inverse_variable_map;

   public:
    plan_to_cnf_map();
    ~plan_to_cnf_map();

    // These methos get the information about a variable/action from a planning problem and return
    // the variable index into the cnf formula
    // They also add is to the pool of variables
    // This method is used for planning variables: tag, timestep, var_index, value
    int get_variable_index(variable_tag tag, int timestep, int var_index, int value);
    // This method is used for planning operators and helper variables: tag, timestep, var_index
    int get_variable_index(variable_tag tag, int timestep, int var_index);

    // same as above but does not add if variable does not exists. Returns -1 in this case
    int get_variable_index_without_adding(variable_tag tag, int timestep, int var_index, int value);
    int get_variable_index_without_adding(variable_tag tag, int timestep, int var_index);

    // inverse of the methods above
    tagged_variable get_planning_info_for_variable(int index);

    // calculates the permutation between two timesteps. All variables have to already be present for this
    // advances the timestep of every variable by t_diff
    std::vector<int> calculate_permutation_by_timesteps(int t_diff, int num_timesteps);

    int get_num_variables();
};
}  // namespace planning_logic