#include "plan_to_cnf_map.h"

#include "logging.h"
namespace planning_logic {

int plan_to_cnf_map::get_variable_index(variable_tag tag, int timestep, int var_index, int value) {
    tagged_variable var = std::make_tuple(tag, timestep, var_index, value);
    if (m_variable_map.find(var) == m_variable_map.end()) {
        int size = m_variable_map.size();
        m_variable_map[var] = size + 1;
    }

    // also add information to the inverted variable map
    m_inverse_variable_map[m_variable_map[var]] = var;
    return m_variable_map[var];
}

int plan_to_cnf_map::get_variable_index_without_adding(variable_tag tag, int timestep, int var_index, int value) {
    tagged_variable var = std::make_tuple(tag, timestep, var_index, value);
    if (m_variable_map.find(var) == m_variable_map.end()) {
        return -1;
    }
    return m_variable_map[var];
}

int plan_to_cnf_map::get_variable_index(variable_tag tag, int timestep, int var_index) {
    return get_variable_index(tag, timestep, var_index, 0);
}

int plan_to_cnf_map::get_variable_index_without_adding(variable_tag tag, int timestep, int var_index) {
    return get_variable_index_without_adding(tag, timestep, var_index, 0);
}

tagged_variable plan_to_cnf_map::get_planning_info_for_variable(int index) {
    if (m_inverse_variable_map.find(index) == m_inverse_variable_map.end()) {
        return std::make_tuple(variable_none, -1, -1, -1);
    }
    return m_inverse_variable_map[index];
}

std::vector<int> plan_to_cnf_map::calculate_permutation_by_timesteps(int t_diff, int num_timesteps) {
    LOG_MESSAGE(log_level::info) << "Calculating permutation of size " << m_variable_map.size() + 1
                                 << " for t_diff=" << t_diff;

    std::vector<int> from_to_index(m_variable_map.size() + 1);
    from_to_index[0] = 0;  // dummy variable does not get permuted

    // calculate the mapping
    for (std::map<tagged_variable, int>::iterator iter = m_variable_map.begin(); iter != m_variable_map.end(); ++iter) {
        tagged_variable tagged_var = iter->first;
        variable_tag tag = std::get<0>(tagged_var);
        int t = std::get<1>(tagged_var);
        int t_to;

        // also allow negative t_diffs
        if (tag == variable_plan_var) {
            // variables are relevant for t+1 timesteps
            int modulus = num_timesteps + 1;
            t_to = (modulus + t + t_diff) % modulus;
        }
        if (tag == variable_plan_op) {
            // operators are only relevant for t timesteps
            int modulus = num_timesteps;
            t_to = (modulus + t + t_diff) % modulus;
        }

        tagged_variable tagged_var_to = tagged_var;
        std::get<1>(tagged_var_to) = t_to;
        from_to_index[m_variable_map[tagged_var]] = m_variable_map[tagged_var_to];
    }

    return from_to_index;
}
}  // namespace planning_logic