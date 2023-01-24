#include "dd_builder_topk.h"

#include "bdd_container.h"
#include "dd_builder.h"
#include "logic_primitive.h"
#include "variable_creation.h"
#include "dd_builder_variable_order.h"

#include <fstream>

using namespace encoder;

namespace dd_builder {

void construct_dd_top_k(dd_buildable &container, encoder_abstract &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Running TopK Configuration";

    int min_plan_length = options.timesteps;
    LOG_MESSAGE(log_level::info) << "Using a minimal planlength of " << min_plan_length;
    // NOTE(JP) it seems to make a difference, if i create every variables at once or only some at a time.
    // this is quiet wonky. i have no clue if it actually helps or is decresing the performance
    // NOTE(JP) this seems to make it impossible to conjoin bdd with goal in the end. it is so wonky
     variable_creation::create_variables_for_first_t_steps(min_plan_length, encoder, container, options);

    container.set_num_dds(2);

    // split the order into parts (in a really complicated manner)
    // default rympec::
    std::stringstream ss(options.build_order);
    std::string order;
    std::getline(ss, order, ':');

    // construct initial state
    // NOTE(JP) creating the vraiable here does not help, this approach will stay weaker on some testcases
    // variable_creation::create_variables_for_timestep_t(0, encoder, container, options);
    std::vector<planning_logic::logic_primitive> temp = encoder.get_logic_primitives(planning_logic::ini_state, 0);
    conjoin_primitives_linear(container, temp, 0, true);

    double total_number_of_plans = 0;
    int current_timestep = 0;
    while (true) {
        // calculate new solutions if we dont use a fd run before or if we reached the minimal plan length
        if (current_timestep >= min_plan_length) {
            // calculate number of new plans
            conjoin_main_dd_with_goal(container, encoder, 0, 1, current_timestep);
            double local_number_of_plans = container.count_num_solutions(1);
            container.clear_dd(1);
            total_number_of_plans += local_number_of_plans;
            LOG_MESSAGE(log_level::info) << "Found " << local_number_of_plans << " new plans in timestep "
                                         << current_timestep << " new total is: " << total_number_of_plans;
        }
        if (total_number_of_plans > options.num_plans) {
            break;
        }

        // TODO abort if there are not enough plans
        // NOTE(JP) creating the vraiable here does not help, this approach will stay waeaker on some testcases
        variable_creation::create_variables_for_timestep_t(current_timestep+1, encoder, container, options);
        // extend new timestep
        LOG_MESSAGE(log_level::info) << "Extending timestep " << current_timestep;
        // NOTE: it did not help to try to only include eo_op once
        temp = conjoin_order::order_clauses_for_layer(encoder, order, current_timestep);
        LOG_MESSAGE(log_level::info) << "Conjoining clauses " << current_timestep;
        conjoin_primitives_linear(container, temp, 0, true);

        // updating current timestep
        current_timestep++;
    }
}

void construct_dd_top_k_restarting(encoder_abstract &encoder, option_values &options) {
    int curr_timestep = options.timesteps;
    int max_timesteps = -1;
    if (options.quality_bound >= 1.0) {
        max_timesteps = options.quality_bound * options.timesteps;
    }

    double total_plans = 0.0;
    while (true) {
        bdd_container container(1);

        option_values tmp_opts = options;
        tmp_opts.timesteps = curr_timestep;

        // variable order
        variable_creation::create_variables_for_first_t_steps(tmp_opts.timesteps, encoder, container, tmp_opts);
        std::vector<int> var_order = variable_order::order_variables(encoder, tmp_opts);
        container.set_variable_order(var_order);

        // reordering
        if (tmp_opts.no_reordering) {
            container.disable_reordering();
        } else {
            container.enable_reordering();
        }

        construct_dd_linear(container, encoder, tmp_opts, true);

        double curr_plans = container.count_num_solutions(0);
        total_plans += curr_plans;

        LOG_MESSAGE(log_level::info) << "Found " << curr_plans << " new plans in timestep " << curr_timestep
                                     << " new total is: " << total_plans;

        if (tmp_opts.quality_bound >= 1.0) {
            // write solution to file in the case of top-q planning
            LOG_MESSAGE(log_level::info) << "Writing BDD to file";
            container.write_bdd_to_dot_file("bdd_step_" + std::to_string(curr_timestep) + ".dot");
        }

        if (tmp_opts.quality_bound >= 1.0 && curr_timestep >= max_timesteps) {
            // terminate for the case of top-q planning
            return;
        }
        if (tmp_opts.num_plans >= 0 && total_plans >= tmp_opts.num_plans) {
            return;
        }

        curr_timestep++;
    }
}

void construct_dd_top_k_with_all_goals(dd_buildable &container, encoder_abstract &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Running TopK Configuration with prebuild goals";

    container.set_num_dds(1);

    // split the order into parts (in a really complicated manner)
    // default rympec::
    std::stringstream ss(options.build_order);
    std::string order;
    std::getline(ss, order, ':');

    // the 100 is a realy magic number
    // TODO tune it
    const int num_prebuild_timesteps = 20;

    // create all variables
    variable_creation::create_variables_for_first_t_steps(num_prebuild_timesteps, encoder, container, options);

    // prebuild the goal
    std::vector<planning_logic::logic_primitive> prebuild_goals = encoder.prebuild_goals(num_prebuild_timesteps);
    conjoin_primitives_linear(container, prebuild_goals, 0, true);

    // construct initial state
    std::vector<planning_logic::logic_primitive> ini_seed = encoder.get_logic_primitives(planning_logic::ini_state, 0);
    conjoin_primitives_linear(container, ini_seed, 0, true);

    double total_number_of_plans = 0;
    int current_timestep = 0;
    while (true) {
        // calculate number of new plans
        // total_number_of_plans = container.count_num_solutions(0);
        // LOG_MESSAGE(log_level::info) << "Found " << total_number_of_plans << " plans in timesteps " <<
        // current_timestep;
        if (total_number_of_plans > options.num_plans) {
            break;
        }

        // extend new timestep
        LOG_MESSAGE(log_level::info) << "Extending timestep " << current_timestep;
        std::vector<planning_logic::logic_primitive> current_step =
            conjoin_order::order_clauses_for_layer(encoder, order, current_timestep);
        conjoin_primitives_linear(container, current_step, 0, true);

        // updating current timestep
        current_timestep++;
    }
}
}  // namespace dd_builder