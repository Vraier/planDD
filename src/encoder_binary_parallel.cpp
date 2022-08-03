#include "encoder_binary_parallel.h"

using namespace planning_logic;

namespace encoder {

binary_parallel::binary_parallel(option_values &options, sas_problem &problem, graph::undirected_graph &conflict_graph)
    : m_symbol_map(problem.m_operators.size()),
      m_sas_problem(problem),
      m_options(options),
      m_action_conflicts(conflict_graph) {
    m_action_conflicts = conflict_graph;
    graph::undirected_graph complement = m_action_conflicts.construct_complement();
    m_colouring = graph::approximate_colouring(complement);

    m_num_colours = 0;
    for (int i = 0; i < m_colouring.size(); i++) {
        m_num_colours = m_colouring[i] > m_num_colours ? m_colouring[i] : m_num_colours;
    }
    m_num_colours++;

    m_colour_class_size = std::vector<int>(m_num_colours);
    for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
        int op_col = m_colouring[op];
        m_group_id[op] = m_colour_class_size[m_colouring[op]];
        m_colour_class_size[m_colouring[op]]++;
    }

    // increase every colour class by one to allow a 'noop' action
    for (int i = 0; i < m_colour_class_size.size(); i++) {
        m_colour_class_size[i]++;
    }
}

std::vector<logic_primitive> binary_parallel::construct_initial_state() {
    std::vector<logic_primitive> result;

    for (int var = 0; var < m_sas_problem.m_variabels.size(); var++) {
        int var_size = m_sas_problem.m_variabels[var].m_range;
        int var_val = m_sas_problem.m_initial_state[var];
        std::vector<int> var_encoding =
            m_symbol_map.get_variable_index_binary(variable_plan_binary_var, 0, var, var_val, var_size);
        std::vector<std::vector<int>> new_dnf;
        new_dnf.push_back(var_encoding);
        result.push_back(logic_primitive(logic_dnf, ini_state, 0, new_dnf));
    }

    return result;
}

std::vector<logic_primitive> binary_parallel::construct_goal(int timestep) {
    std::vector<logic_primitive> result;

    for (int g = 0; g < m_sas_problem.m_goal.size(); g++) {
        std::pair<int, int> goal_pair = m_sas_problem.m_goal[g];
        int goal_var = goal_pair.first;
        int goal_val = goal_pair.second;
        int goal_var_size = m_sas_problem.m_variabels[goal_var].m_range;

        std::vector<int> goal_encoding = m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep,
                                                                                goal_var, goal_val, goal_var_size);
        std::vector<std::vector<int>> new_dnf;
        new_dnf.push_back(goal_encoding);
        result.push_back(logic_primitive(logic_dnf, goal, 0, new_dnf));
    }

    return result;
}

std::vector<logic_primitive> binary_parallel::construct_no_impossible_value(int timestep) {
    std::vector<logic_primitive> result;

    for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
        // iterate over the indizes that represent imposssible variable values
        int var_size = m_sas_problem.m_variabels[v].m_range;
        int num_imp_vars = (1 << m_symbol_map.num_bits_for_binary_var(var_size));
        for (int imp_var = var_size; imp_var < num_imp_vars; imp_var++) {
            std::vector<int> var_indizes =
                m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep, v, imp_var, var_size);
            std::vector<int> new_clause;
            for (int i : var_indizes) {
                new_clause.push_back(-i);
            }
            result.push_back(logic_primitive(logic_clause, eo_var, timestep, new_clause));
        }
    }
    return result;
}

std::vector<logic_primitive> binary_parallel::construct_exact_one_action(int timestep) {
    std::vector<logic_primitive> result;

    for (int op1 = 0; op1 < m_sas_problem.m_operators.size(); op1++) {
        for (int op2 = op1 + 1; op2 < m_sas_problem.m_operators.size(); op2++) {
            int col1 = m_colouring[op1];
            int col2 = m_colouring[op2];

            if (col1 == col2) {
                // operators are mutually exclusive anyways. Don't have to do
                // anything
                continue;
            }

            if (m_action_conflicts.are_neighbours(op1, op2)) {
                std::vector<int> op_enc1 = m_symbol_map.get_variable_index_binary(
                    variable_plan_binary_op, timestep, col1, m_group_id[op1], m_colour_class_size[col1]);
                std::vector<int> op_enc2 = m_symbol_map.get_variable_index_binary(
                    variable_plan_binary_op, timestep, col2, m_group_id[op2], m_colour_class_size[col2]);

                std::vector<int> new_clause;
                for (int i = 0; i < op_enc1.size(); i++) {
                    new_clause.push_back(-op_enc1[i]);
                }
                for (int i = 0; i < op_enc2.size(); i++) {
                    new_clause.push_back(-op_enc2[i]);
                }

                result.push_back(logic_primitive(logic_clause, eo_op, timestep, new_clause));
            }
        }
    }

    // TODO: guarantee that at least one action is taken in every timestep

    if (m_options.binary_exclude_impossible) {
        // exclude the impossible actions for every colour class
        for (int col = 0; col < m_num_colours; col++) {
            // iterate over the indizes that represent imposssible operators
            int num_imp_ops = (1 << m_symbol_map.num_bits_for_binary_var(m_colour_class_size[col]));
            for (int imp_op = m_colour_class_size[col]; imp_op < num_imp_ops; imp_op++) {
                std::vector<int> op_indizes = m_symbol_map.get_variable_index_binary(
                    variable_plan_binary_op, timestep, col, imp_op, m_colour_class_size[col]);
                std::vector<int> new_clause;
                for (int i : op_indizes) {
                    new_clause.push_back(-i);
                }
                result.push_back(logic_primitive(logic_clause, eo_op, timestep, new_clause));
            }
        }
    }
}

std::vector<logic_primitive> binary_parallel::construct_mutex(int timestep) {
    std::vector<logic_primitive> result;

    if (!m_options.include_mutex) {
        return result;
    }

    for (int m = 0; m < m_sas_problem.m_mutex_groups.size(); m++) {
        std::vector<std::pair<int, int>> at_most_one_should_be_true = m_sas_problem.m_mutex_groups[m];

        for(int i = 0; i < at_most_one_should_be_true.size(); i++){
            for(int j = i+1; j < at_most_one_should_be_true.size(); j++){
                int var1, val1, size1, var2, val2, size2;
                var1 = at_most_one_should_be_true[i].first;
                val1 = at_most_one_should_be_true[i].second;
                size1 = m_sas_problem.m_variabels[var1].m_range;
                var2 = at_most_one_should_be_true[j].first;
                val2 = at_most_one_should_be_true[j].second;
                size2 = m_sas_problem.m_variabels[var2].m_range;

                std::vector<int> new_clause;
                // binary planning variables
                for(int v: m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep, var1, val1, size1)){
                    new_clause.push_back(-v);
                }
                for(int v: m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep, var2, val2, size2)){
                    new_clause.push_back(-v);
                }
                result.push_back(logic_primitive(logic_clause, mutex, timestep, new_clause));
            }
        }
    }

    return result;
}

std::vector<logic_primitive> binary_parallel::construct_precondition(int timestep) {}

std::vector<logic_primitive> binary_parallel::construct_effect(int timestep) {}

std::vector<logic_primitive> binary_parallel::construct_frame(int timestep) {}

}  // namespace encoder