#include "dd_builder.h"

#include "logging.h"

using namespace planning_logic;

namespace dd_builder {

void construct_dd_clause_linear(dd_buildable &dd, std::vector<conjoin_order::tagged_logic_primitiv> &logic_primitives,
                                int dd_index, bool silent) {
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
            if(!silent){
                LOG_MESSAGE(log_level::info) << "Conjoined " << percent
                                             << "% of all clauses. " + dd.get_short_statistics(dd_index);
            }
        }
    }

    LOG_MESSAGE(log_level::info) << "Finished constructing DD";
}

void construct_bdd_by_layer(bdd_container &bdd, formula &cnf, option_values &options) {

    std::string build_order = options.build_order;
    if(!conjoin_order::is_valid_layer_order_string(build_order)){
        LOG_MESSAGE(log_level::error) << "Build order string is not compatible wit layer building";
        return;
    }

    // split the order into parts (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string ini_seed, layer_seed, goal_seed;
    std::getline(ss, ini_seed, ':');
    std::getline(ss, layer_seed, ':');
    std::getline(ss, goal_seed, ':');

    // indizes of the different bdds
    const int main_bdd_idx = 0;
    std::vector<int> layer_bdd_idxs(options.timesteps);
    // permute variables by one timestep
    std::vector<int> permutation = cnf.calculate_permutation_by_timesteps(1);

    // build the initial layer bdds
    LOG_MESSAGE(log_level::info) << "Start building layer BDDs";
    // constructs the first layer
    const int first_layer = 0;
    layer_bdd_idxs[first_layer] = 2;
    std::vector<conjoin_order::tagged_logic_primitiv> layer_primitives =
    conjoin_order::order_clauses_for_layer(cnf, first_layer, layer_seed);
    construct_dd_clause_linear(bdd, layer_primitives, layer_bdd_idxs[first_layer], true);

    // construct the layer bdds only when needed
    if(options.layer_on_the_fly){
        for(int i = 1; i < options.timesteps; i++){
            layer_bdd_idxs[i] = 2; // only need one bdd slot
        }
        // no bdds need to be constructed
        bdd.reduce_heap();
    } 
    // construct all layer bdds at the beginning
    else {
        for(int i = 1; i < options.timesteps; i++){
            layer_bdd_idxs[i] = i+2;
            // construct new bdd form sccratch or by permutating the variables of the predecessor
            if(options.use_layer_permutation){
                bdd.permute_variables(permutation, layer_bdd_idxs[i-1], layer_bdd_idxs[i]);
            }
            else {
                std::vector<conjoin_order::tagged_logic_primitiv> layer_primitives =
                conjoin_order::order_clauses_for_layer(cnf, i, layer_seed);
                construct_dd_clause_linear(bdd, layer_primitives, layer_bdd_idxs[i], true);
            }
        }

        bdd.reduce_heap();
    }

    // TODO think about variable ordering
    // apply the extended variable map to the main bdd
    // single_step_bdd.set_variable_order(order_for_all_timesteps);
    // main_bdd.set_variable_order(order_for_all_timesteps);

    // build the bdd for the foundation bdd
    LOG_MESSAGE(log_level::info) << "Start building foundation BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> foundation_primitives =
        conjoin_order::order_clauses_for_foundation(cnf, ini_seed);
    construct_dd_clause_linear(bdd, foundation_primitives, main_bdd_idx, true);

    // build the main bdd layer by layer
    LOG_MESSAGE(log_level::info) << "Adding timestep for t=" << first_layer << " " << bdd.get_short_statistics(main_bdd_idx);
    bdd.conjoin_two_bdds(main_bdd_idx, layer_bdd_idxs[first_layer], main_bdd_idx);
    for (int t = 1; t < options.timesteps; t++) {
        LOG_MESSAGE(log_level::info) << "Adding timestep for t=" << t << " " << bdd.get_short_statistics(main_bdd_idx);
        if(options.layer_on_the_fly){
            // construct new needed bdd
            if(options.use_layer_permutation){
                bdd.permute_variables(permutation, layer_bdd_idxs[t-1], layer_bdd_idxs[t]);
            } else {
                bdd.clear_bdd(layer_bdd_idxs[t]);
                std::vector<conjoin_order::tagged_logic_primitiv> layer_primitives =
                conjoin_order::order_clauses_for_layer(cnf, t, layer_seed);
                construct_dd_clause_linear(bdd, layer_primitives, layer_bdd_idxs[t], true);
            }
        }
        bdd.conjoin_two_bdds(main_bdd_idx, layer_bdd_idxs[t], main_bdd_idx);
    }

    LOG_MESSAGE(log_level::info) << "Finished conjoining all timesteps";

    return;
}

void construct_bdd_by_layer_bidirectional(bdd_container &bdd, formula &cnf, option_values &options) {
    const int main_begin_idx = 0;
    const int main_end_idx = 1;
    const int init_step_bdd_idx = 2;
    const int goal_step_bdd_idx = 3;

    // split the order into parts (in a really complicated manner)
    std::stringstream ss(options.build_order);
    std::string ini_seed, layer_seed, goal_seed;
    std::getline(ss, ini_seed, ':');
    std::getline(ss, layer_seed, ':');
    std::getline(ss, goal_seed, ':');

    // build the bdd for a single timestep
    LOG_MESSAGE(log_level::info) << "Start building init step BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> init_step_primitives =
        conjoin_order::order_clauses_for_layer(cnf, 0, layer_seed);
    construct_dd_clause_linear(bdd, init_step_primitives, init_step_bdd_idx);
    bdd.reduce_heap();
    LOG_MESSAGE(log_level::info) << "Start building goal step BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> goal_step_primitives =
        conjoin_order::order_clauses_for_layer(cnf, options.timesteps-1, layer_seed);
    construct_dd_clause_linear(bdd, goal_step_primitives, goal_step_bdd_idx);
    bdd.reduce_heap();

    // apply the extended variable map to the main bdd
    // single_step_bdd.set_variable_order(order_for_all_timesteps);
    // main_bdd.set_variable_order(order_for_all_timesteps);

    // build the bdd for no timestep
    LOG_MESSAGE(log_level::info) << "Start building init foundation BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> init_foundation_primitives =
        conjoin_order::order_clauses_for_foundation(cnf, ini_seed);
    construct_dd_clause_linear(bdd, init_foundation_primitives, main_begin_idx);
    LOG_MESSAGE(log_level::info) << "Start building goal foundation BDD";
    std::vector<conjoin_order::tagged_logic_primitiv> goal_foundation_primitives =
        conjoin_order::order_clauses_for_foundation(cnf, goal_seed);
    construct_dd_clause_linear(bdd, goal_foundation_primitives, main_end_idx);

    int t_begin = 0;
    int t_end = options.timesteps - 1;
    bool forward_step = true;
    std::vector<int> forward_permutation = cnf.calculate_permutation_by_timesteps(1);
    std::vector<int> backward_permutation = cnf.calculate_permutation_by_timesteps(-1);

    while (t_begin <= t_end) {
        if (forward_step) {
            LOG_MESSAGE(log_level::info) << "Adding forward timestep for t=" << t_begin << " "
                                         << bdd.get_short_statistics(main_begin_idx);
            bdd.conjoin_two_bdds(main_begin_idx, init_step_bdd_idx, main_begin_idx);
            bdd.permute_variables(forward_permutation, init_step_bdd_idx, init_step_bdd_idx);
            t_begin++;
            forward_step = false;
        } else {
            LOG_MESSAGE(log_level::info) << "Adding backward timestep for t=" << t_end << " "
                                         << bdd.get_short_statistics(main_end_idx);
            bdd.conjoin_two_bdds(main_end_idx, goal_step_bdd_idx, main_end_idx);
            bdd.permute_variables(backward_permutation, goal_step_bdd_idx, goal_step_bdd_idx);
            t_end--;
            forward_step = true;
        }
    }
    LOG_MESSAGE(log_level::info) << "Doing final conjoin";
    bdd.conjoin_two_bdds(main_begin_idx, main_end_idx, main_begin_idx);

    LOG_MESSAGE(log_level::info) << "Finished conjoining all timesteps";

    return;
}

}  // namespace dd_builder