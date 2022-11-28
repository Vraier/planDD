#include "dd_builder_topk.h"

#include <regex>

#include "bdd_container.h"
#include "dd_builder.h"
#include "logic_primitive.h"
#include "variable_creation.h"

#include <fstream>

using namespace encoder;

namespace dd_builder {

// finds the legth of the plan from an fast downward output file
int get_plan_length(std::string file_path){
    std::ifstream infile(file_path);
    std::string line;

    while (std::getline(infile, line)){
        std::regex rgx("\\[.*\\] Plan length: ([0-9]*) step\\(s\\).");
        std::smatch match;
        const std::string constLine = line;
        if(std::regex_search(constLine.begin(), constLine.end(), match, rgx)){
            return std::stoi(match[1].str().c_str());
        }
    }
    return -1;
}


void construct_dd_top_k(dd_buildable &container, encoder_abstract &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Running TopK Configuration";

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
        if (total_number_of_plans > options.num_plans) {
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


void construct_dd_top_k_restarting(encoder_abstract &encoder, option_values &options) {
    int curr_timestep = 0;  // TODO, maybe start at 1

    if(options.use_fd){
        curr_timestep = get_plan_length("fd_output.txt");
        if(curr_timestep < 0){
            LOG_MESSAGE(log_level::error) << "Could not extract minimal planlength";
            return;
        } else {
            LOG_MESSAGE(log_level::error) << "Extracted a minimal planlegth of " << curr_timestep;
        }
    }

    double total_plans = 0.0;
    while (true) {
        option_values temp_opts = options;
        temp_opts.timesteps = curr_timestep;
        bdd_container container(1);
        construct_dd_linear(container, encoder, temp_opts, true);

        double curr_plans = container.count_num_solutions(0);
        total_plans += curr_plans;

        LOG_MESSAGE(log_level::info) << "Found " << curr_plans << " new plans in timestep " << curr_timestep
                                     << " new total is: " << total_plans;

        if(total_plans >= options.num_plans){
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