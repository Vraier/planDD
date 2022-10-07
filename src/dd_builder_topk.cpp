#include "dd_builder_topk.h"

#include "dd_builder.h"
#include "logic_primitive.h"

namespace dd_builder {
void construct_dd_top_k(dd_buildable &container, encoder::encoder_abstract &encoder, option_values &options) {
    container.set_num_dds(2);

    // split the order into parts (in a really complicated manner)
    // default rympec::
    std::stringstream ss(options.build_order);
    std::string order;
    std::getline(ss, order, ':');

    // construct initial state
    std::vector<planning_logic::logic_primitive> temp = encoder.get_logic_primitives(planning_logic::ini_state, 0);
    conjoin_primitives_linear(container, temp, 0, true);

    double total_number_of_plans = 0;
    int current_timestep = 0;
    while (true) {
        
        // calculate number of new plans
        conjoin_main_dd_with_goal(container, encoder, 0, 1, current_timestep);
        double local_number_of_plans = container.count_num_solutions(1);
        container.clear_dd(1);
        total_number_of_plans += local_number_of_plans;
        LOG_MESSAGE(log_level::info) << "Found " << local_number_of_plans << " new plans in timestep "
                                     << current_timestep << " new total is: " << total_number_of_plans;
        if(total_number_of_plans > options.num_plans){
            break;
        }

        // TODO abbruchkriterium, wenn nicht gen"ugend pl"ane existieren
        // extend new timestep
        LOG_MESSAGE(log_level::info) << "Extending timestep " << current_timestep;
        // NOTE: it did not help to try to only include eo_op once
        temp = conjoin_order::order_clauses_for_layer(encoder, order, current_timestep);
        conjoin_primitives_linear(container, temp, 0, true);

        // updating current timestep
        current_timestep++;
    }
}
}  // namespace dd_builder