#include "dd_builder.h"

#include "logging.h"

using namespace planning_logic;

namespace dd_builder {

void construct_dd_clause_linear(dd_buildable &dd, std::vector<conjoin_order::tagged_logic_primitiv> &logic_primitives,
                                int dd_index) {
    LOG_MESSAGE(log_level::info) << "Start constructing DD";

    // conjoin the clauses in the correct order
    int percent = 0;
    for (int i = 0; i < logic_primitives.size(); i++) {
        planning_logic::logic_primitive primitive = logic_primitives[i].first;
        planning_logic::logic_primitive_type primitive_type = logic_primitives[i].second;

        if (primitive_type == logic_clause) {
            dd.conjoin_clause(primitive, dd_index);
        } else if (primitive_type == logic_eo) {
            dd.add_exactly_one_constraint(primitive, dd_index);
        } else {
            LOG_MESSAGE(log_level::warning) << "Unknown logic primitive type during DD construction";
        }

        int new_percent = (100 * (i + 1)) / logic_primitives.size();
        if (new_percent > percent) {
            percent = new_percent;
            LOG_MESSAGE(log_level::info) << "Conjoined " << percent
                                         << "% of all clauses. " + dd.get_short_statistics(dd_index);
        }
    }

    LOG_MESSAGE(log_level::info) << "Finished constructing DD";
}

void construct_bdd_by_layer(bdd_container &bdd, formula &cnf, option_values &options) {
    const int main_bdd_idx = 0;
    const int single_step_bdd_idx = 1;

    // build the bdd for a single timestep
    LOG_MESSAGE(log_level::info) << "Start building single step BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> single_step_primitives =
        conjoin_order::order_clauses_for_layer(cnf, 0);
    construct_dd_clause_linear(bdd, single_step_primitives, single_step_bdd_idx);
    bdd.reduce_heap();

    // apply the extended variable map to the main bdd
    // single_step_bdd.set_variable_order(order_for_all_timesteps);
    // main_bdd.set_variable_order(order_for_all_timesteps);

    // build the bdd for no timestep
    LOG_MESSAGE(log_level::info) << "Start building no step BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> no_step_primitives =
        conjoin_order::order_clauses_for_foundation(cnf);
    construct_dd_clause_linear(bdd, no_step_primitives, main_bdd_idx);

    // build the main bdd layer by layer
    // copying the bdd from the single step one to the main bdd
    std::vector<int> permutation = cnf.calculate_permutation_by_timesteps(1);
    for (int t = 0; t < options.timesteps; t++) {
        LOG_MESSAGE(log_level::info) << "Adding timestep for t=" << t << " " << bdd.get_short_statistics(main_bdd_idx);
        bdd.conjoin_two_bdds(main_bdd_idx, single_step_bdd_idx, main_bdd_idx);
        bdd.permute_variables(permutation, single_step_bdd_idx, single_step_bdd_idx);
    }

    LOG_MESSAGE(log_level::info) << "Finished conjoining all timesteps";

    return;
}
}  // namespace dd_builder