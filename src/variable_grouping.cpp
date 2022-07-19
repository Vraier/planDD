#include "variable_grouping.h"

using namespace planning_logic;

namespace variable_grouping {

void create_all_variables(cnf_encoder &encoder, bdd_container &container, option_values &options) {
    container.disable_reordering();

    encoder.m_symbol_map.set_num_operators(encoder.m_sas_problem.m_operators.size());
    for (int t = 0; t <= options.timesteps; t++) {
        // construct all variables indizes
        int var_group_start = encoder.m_symbol_map.next_used_index();
        int var_group_size;
        for (int var = 0; var < encoder.m_sas_problem.m_variabels.size(); var++) {
            for (int val = 0; val < encoder.m_sas_problem.m_variabels[var].m_range; val++) {
                encoder.m_symbol_map.get_variable_index(variable_plan_var, t, var, val);
            }
        }
        if (options.group_variables) {
            container.set_variable_group(var_group_start, var_group_size);
        }

        // construct action indizes
        if (t != options.timesteps) {
            int op_group_start = encoder.m_symbol_map.next_used_index();
            int op_group_size;
            if (options.binary_encoding) {
                encoder.m_symbol_map.get_variable_index_for_op_binary(t, 0);
            } else {
                for (int op = 0; op < encoder.m_sas_problem.m_operators.size(); op++) {
                    encoder.m_symbol_map.get_variable_index(variable_plan_op, t, op);
                }
            }
            op_group_size = encoder.m_symbol_map.next_used_index() - op_group_start;

            if (options.group_actions) {
                container.set_variable_group(op_group_start, op_group_size);
            }
        }
    }

    container.enable_reordering();
    LOG_MESSAGE(log_level::info) << "Constructed " << encoder.m_symbol_map.get_num_variables()
                                 << " variables during symbol map initialization";
}
}  // namespace variable_grouping