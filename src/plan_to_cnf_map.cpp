#include "plan_to_cnf_map.h"

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