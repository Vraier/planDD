#include "encoder_binary_parallel.h"
#include "logging.h"

using namespace planning_logic;

namespace encoder {

binary_parallel::binary_parallel(option_values &options, sas_problem &problem, graph::undirected_graph &conflict_graph)
    : encoder_abstract(options, problem, problem.m_operators.size()), m_action_conflicts(conflict_graph) {
    graph::undirected_graph complement = m_action_conflicts.construct_complement();
    
    m_group_id = std::vector<int>(m_sas_problem.m_operators.size());

    m_colouring = graph::approximate_colouring(complement);
    m_num_colours = 0;
    for (int i = 0; i < m_colouring.size(); i++) {
        m_num_colours = m_colouring[i] > m_num_colours ? m_colouring[i] : m_num_colours;
    }
    m_num_colours++;

    LOG_MESSAGE(log_level::info) << "Encoder found " << m_num_colours << " colour classes";

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

    LOG_MESSAGE(log_level::info) << "Finished construction binary parallel encoder";
}

std::vector<logic_primitive> binary_parallel::get_logic_primitives(primitive_tag tag, int timestep) {
    update_timesteps(timestep);

    // std::cout << "Num Vars: " << m_symbol_map.get_num_variables() << " timestep: " << timestep << std::endl;

    switch (tag) {
        case ini_state:
            if (timestep != 0) {
                return std::vector<logic_primitive>();
            } else {
                return construct_initial_state();
            }
        case goal:
            return construct_goal(timestep);
        case eo_var:
            return construct_no_impossible_value(timestep);
        case eo_op:
            return construct_exact_one_action(timestep);
        case mutex:
            return construct_mutex(timestep);
        case precon:
            return construct_precondition(timestep);
        case effect:
            return construct_effect(timestep);
        case frame:
            return construct_frame(timestep);
        case none:
        default:
            return std::vector<logic_primitive>();
    }
}

void binary_parallel::update_timesteps(int timestep) {
    m_num_timesteps = timestep > m_num_timesteps ? timestep : m_num_timesteps;
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

    return result;
}

std::vector<logic_primitive> binary_parallel::construct_mutex(int timestep) {
    std::vector<logic_primitive> result;

    if (!m_options.include_mutex) {
        return result;
    }

    for (int m = 0; m < m_sas_problem.m_mutex_groups.size(); m++) {
        std::vector<std::pair<int, int>> at_most_one_should_be_true = m_sas_problem.m_mutex_groups[m];

        for (int i = 0; i < at_most_one_should_be_true.size(); i++) {
            for (int j = i + 1; j < at_most_one_should_be_true.size(); j++) {
                int var1, val1, size1, var2, val2, size2;
                var1 = at_most_one_should_be_true[i].first;
                val1 = at_most_one_should_be_true[i].second;
                size1 = m_sas_problem.m_variabels[var1].m_range;
                var2 = at_most_one_should_be_true[j].first;
                val2 = at_most_one_should_be_true[j].second;
                size2 = m_sas_problem.m_variabels[var2].m_range;

                std::vector<int> new_clause;
                // binary planning variables
                for (int v :
                     m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep, var1, val1, size1)) {
                    new_clause.push_back(-v);
                }
                for (int v :
                     m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep, var2, val2, size2)) {
                    new_clause.push_back(-v);
                }
                result.push_back(logic_primitive(logic_clause, mutex, timestep, new_clause));
            }
        }
    }

    return result;
}

std::vector<logic_primitive> binary_parallel::construct_precondition(int timestep) {
    std::vector<logic_primitive> result;

    for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
        for (int eff = 0; eff < m_sas_problem.m_operators[op].m_effects.size(); eff++) {
            int op_col, effected_var, effected_old_val, effected_var_size;
            op_col = m_colouring[op];
            effected_var = std::get<0>(m_sas_problem.m_operators[op].m_effects[eff]);
            effected_old_val = std::get<1>(m_sas_problem.m_operators[op].m_effects[eff]);
            effected_var_size = m_sas_problem.m_variabels[effected_var].m_range;

            if (effected_old_val == -1) {
                // a value of -1 the value of the variable is irrelevant,
                // when determining if it is applicable
                continue;
            }

            std::vector<std::vector<int>> new_dnf;

            std::vector<int> op_indizes = m_symbol_map.get_variable_index_binary(
                variable_plan_binary_op, timestep, op_col, m_group_id[op], m_colour_class_size[op_col]);
            for (int o : op_indizes) {
                std::vector<int> tmp;
                tmp.push_back(-o);
                new_dnf.push_back(tmp);
            }

            std::vector<int> var_indizes = m_symbol_map.get_variable_index_binary(
                variable_plan_binary_var, timestep, effected_var, effected_old_val, effected_var_size);
            new_dnf.push_back(var_indizes);

            result.push_back(logic_primitive(logic_dnf, precon, timestep, new_dnf));
        }
    }
    return result;
}

std::vector<logic_primitive> binary_parallel::construct_effect(int timestep) {
    std::vector<logic_primitive> result;

    for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
        for (int eff = 0; eff < m_sas_problem.m_operators[op].m_effects.size(); eff++) {
            int op_col, effected_var, effected_new_val, effected_var_size;
            op_col = m_colouring[op];
            effected_var = std::get<0>(m_sas_problem.m_operators[op].m_effects[eff]);
            effected_new_val = std::get<2>(m_sas_problem.m_operators[op].m_effects[eff]);
            effected_var_size = m_sas_problem.m_variabels[effected_var].m_range;

            std::vector<std::vector<int>> new_dnf;

            std::vector<int> op_indizes = m_symbol_map.get_variable_index_binary(
                variable_plan_binary_op, timestep, op_col, m_group_id[op], m_colour_class_size[op_col]);
            for (int o : op_indizes) {
                std::vector<int> tmp;
                tmp.push_back(-o);
                new_dnf.push_back(tmp);
            }

            std::vector<int> var_indizes = m_symbol_map.get_variable_index_binary(
                variable_plan_binary_var, timestep + 1, effected_var, effected_new_val, effected_var_size);
            new_dnf.push_back(var_indizes);

            result.push_back(logic_primitive(logic_dnf, precon, timestep, new_dnf));
        }
    }
    return result;
}

std::vector<logic_primitive> binary_parallel::construct_frame(int timestep) {
    std::vector<logic_primitive> result;

    for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {  // for every variable
        // find all possible value changes of a variable
        int var_size = m_sas_problem.m_variabels[v].m_range;
        for (int val = 0; val < var_size; val++) {
            // find the actions that support this transition.
            // (planning) indizes of the actions that support the change of the variable
            // to become true in the next timestep
            std::vector<int> support_become_true;
            for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
                for (int i = 0; i < m_sas_problem.m_operators[op].m_effects.size(); i++) {
                    std::tuple<int, int, int> op_eff = m_sas_problem.m_operators[op].m_effects[i];
                    int effected_var, val_pre, val_post;
                    effected_var = std::get<0>(op_eff);
                    val_pre = std::get<1>(op_eff);
                    val_post = std::get<2>(op_eff);

                    // check if corrected variable is affected, and it changes to ture
                    if ((effected_var == v) && (val_post == val)) {
                        support_become_true.push_back(op);
                    }
                }
            }

            std::vector<std::vector<int>> new_dnf;
            std::vector<int> old_var_idzs =
                m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep, v, val, var_size);
            std::vector<int> new_var_idzs =
                m_symbol_map.get_variable_index_binary(variable_plan_binary_var, timestep + 1, v, val, var_size);

            new_dnf.push_back(old_var_idzs);
            for (int new_v : new_var_idzs) {
                std::vector<int> temp;
                temp.push_back(-new_v);
                new_dnf.push_back(temp);
            }

            for (int op : support_become_true) {
                int op_col = m_colouring[op];
                new_dnf.push_back(m_symbol_map.get_variable_index_binary(variable_plan_binary_op, timestep, op_col,
                                                                         m_group_id[op], m_colour_class_size[op_col]));
            }

            result.push_back(logic_primitive(logic_dnf, frame, timestep, new_dnf));
        }
    }
    return result;
}
}  // namespace encoder