#include "dd_builder.h"

#include "dd_builder_conjoin_order.h"
#include "logging.h"

using namespace planning_logic;

namespace dd_builder {

void construct_dd_clause_linear(dd_buildable &dd, formula &cnf, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Start constructing DD";

    // sort the clauses (disjoint and interleaved)
    std::vector<conjoin_order::tagged_logic_primitiv> sorted_primitives = conjoin_order::order_clauses(cnf, options);

    // this reverses the order of the clauses. It allows the variables with the highes timesteps to be conjoined first
    if (options.reverse_order) {
        LOG_MESSAGE(log_level::info) << "Reversing order of the logic primitives";
        std::reverse(sorted_primitives.begin(), sorted_primitives.end());
    }

    // conjoin the clauses in the correct order
    int percent = 0;
    for (int i = 0; i < sorted_primitives.size(); i++) {
        planning_logic::logic_primitive primitive = sorted_primitives[i].first;
        planning_logic::logic_primitive_type primitive_type = sorted_primitives[i].second;

        if (primitive_type == logic_clause) {
            dd.conjoin_clause(primitive);
        } else if (primitive_type == logic_eo) {
            dd.add_exactly_one_constraint(primitive);
        } else {
            LOG_MESSAGE(log_level::warning) << "Unknown logic primitive type during DD construction";
        }

        int new_percent = (100 * (i + 1)) / sorted_primitives.size();
        if (new_percent > percent) {
            percent = new_percent;
            LOG_MESSAGE(log_level::info) << "Conjoined " << percent << "% of all clauses. " + dd.get_short_statistics();
        }
    }

    LOG_MESSAGE(log_level::info) << "Finished constructing DD";
}

void construct_bdd_by_layer(bdd_container &main_bdd, bdd_container &single_step_bdd, formula &cnf, option_values &options){
    return;
}
}  // namespace dd_builder