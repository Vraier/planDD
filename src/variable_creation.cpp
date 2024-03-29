#include "variable_creation.h"

using namespace planning_logic;
using namespace encoder;

namespace variable_creation {

void create_variables_for_first_t_steps(int t, encoder::encoder_abstract &encoder, dd_buildable &container,
                                        option_values &options) {
    LOG_MESSAGE(log_level::info) << "Creating the variables (and grouping) for first " << t << " timesteps";
    
    for (int i = 0; i <= t; i++) {
        create_variables_for_timestep_t(i, encoder, container, options);
    }
}

void create_variables_for_timestep_t(int t, encoder::encoder_abstract &encoder, dd_buildable &container,
                                     option_values &options) {
    // create state variables
    int var_group_start = encoder.m_symbol_map.next_used_index();
    int var_group_size;
    for (int var = 0; var < encoder.m_sas_problem.m_variabels.size(); var++) {
        int var_domain_size = encoder.m_sas_problem.m_variabels[var].m_range;
        int var_group_small_start = encoder.m_symbol_map.next_used_index();
        int var_group_small_size;
        if (options.binary_variables) {
            encoder.m_symbol_map.get_variable_index_for_var_binary(t, var, 0, var_domain_size);
        } else {
            for (int val = 0; val < encoder.m_sas_problem.m_variabels[var].m_range; val++) {
                encoder.m_symbol_map.get_variable_index(variable_plan_var, t, var, val);
            }
        }

        container.create_ith_var(encoder.m_symbol_map.next_used_index() - 1);
        var_group_small_size = encoder.m_symbol_map.next_used_index() - var_group_small_start;
        if (options.group_variables_small) {
            container.set_variable_group(var_group_small_start, var_group_small_size);
        }
    }

    container.create_ith_var(encoder.m_symbol_map.next_used_index() - 1);
    var_group_size = encoder.m_symbol_map.next_used_index() - var_group_start;
    if (options.group_variables) {
        container.set_variable_group(var_group_start, var_group_size);
    }

    // construct action indizes
    if (t != 0) {
        int op_group_start = encoder.m_symbol_map.next_used_index();
        int op_group_size;
        if (options.binary_encoding) {
            encoder.m_symbol_map.get_variable_index_for_op_binary(t - 1, 0);
        } else {
            for (int op = 0; op < encoder.m_sas_problem.m_operators.size(); op++) {
                encoder.m_symbol_map.get_variable_index(variable_plan_op, t-1, op);
            }
        }

        container.create_ith_var(encoder.m_symbol_map.next_used_index() - 1);
        op_group_size = encoder.m_symbol_map.next_used_index() - op_group_start;
        if (options.group_actions) {
            container.set_variable_group(op_group_start, op_group_size);
        }
    }
}

}  // namespace variable_creation