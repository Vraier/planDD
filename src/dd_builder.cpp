#include "dd_builder.h"

#include "logging.h"

using namespace planning_logic;

namespace dd_builder {

void construct_dd_clause_linear(dd_buildable &dd, std::vector<conjoin_order::tagged_logic_primitiv> &logic_primitives) {
    LOG_MESSAGE(log_level::info) << "Start constructing DD";

    // conjoin the clauses in the correct order
    int percent = 0;
    for (int i = 0; i < logic_primitives.size(); i++) {
        planning_logic::logic_primitive primitive = logic_primitives[i].first;
        planning_logic::logic_primitive_type primitive_type = logic_primitives[i].second;

        if (primitive_type == logic_clause) {
            dd.conjoin_clause(primitive);
        } else if (primitive_type == logic_eo) {
            dd.add_exactly_one_constraint(primitive);
        } else {
            LOG_MESSAGE(log_level::warning) << "Unknown logic primitive type during DD construction";
        }

        int new_percent = (100 * (i + 1)) / logic_primitives.size();
        if (new_percent > percent) {
            percent = new_percent;
            LOG_MESSAGE(log_level::info) << "Conjoined " << percent << "% of all clauses. " + dd.get_short_statistics();
        }
    }

    LOG_MESSAGE(log_level::info) << "Finished constructing DD";
}

void construct_bdd_by_layer(bdd_container &main_bdd, bdd_container &single_step_bdd, formula &cnf,
                            option_values &options) {

    // build the bdd for a single timestep
    LOG_MESSAGE(log_level::info) << "Start building single step BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> single_step_primitives =
        conjoin_order::order_clauses_for_single_timestep(cnf, options);
    construct_dd_clause_linear(single_step_bdd, single_step_primitives);
    single_step_bdd.reduce_heap();

    // extend the order to all timesteps in the main bdd
    LOG_MESSAGE(log_level::info) << "Calculating and extending new variable order";
    std::map<int, int> single_step_var_order = single_step_bdd.get_variable_order_for_single_step(cnf.m_variable_map);
    std::vector<int> order_for_all_timesteps =
        single_step_bdd.extend_variable_order_to_all_steps(cnf.m_variable_map, single_step_var_order); 

    // std::vector<int> single_step_order = single_step_bdd.get_variable_order();
    // for (int i = 0; i < order_for_all_timesteps.size(); i++) {
    //     std::cout << "index: " << i << " old layer: " << single_step_order[i]
    //               << " new layer: " << order_for_all_timesteps[i] << std::endl;
    // }

    // apply the extended variable map to the main bdd
    //single_step_bdd.set_variable_order(order_for_all_timesteps);
    //main_bdd.set_variable_order(order_for_all_timesteps);

    // build the bdd for no timestep
    LOG_MESSAGE(log_level::info) << "Start building no step BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> no_step_primitives =
        conjoin_order::order_clauses_for_no_timestep(cnf, options);
    construct_dd_clause_linear(main_bdd, no_step_primitives);

    // build the main bdd layer by layer
    // copying the bdd from the single step one to the main bdd
    for (int t = 0; t < options.timesteps; t++) {
        LOG_MESSAGE(log_level::info) << "Conjoining single step bdd for timestep " << t << " "
                                     << main_bdd.get_short_statistics();
        main_bdd.copy_and_conjoin_bdd_from_another_container(single_step_bdd);
        single_step_bdd.swap_variables_to_other_timestep(cnf.m_variable_map, 1, options.timesteps);

        // single_step
        // a0 -> a1
        // b0 -> b1
        // c0 -> c1

        // single_step_bdd.swap_variables_to_other_timestep(cnf.m_variable_map, t, 0);
    }

    LOG_MESSAGE(log_level::info) << "Finished conjoining all timesteps";

    return;
}
}  // namespace dd_builder