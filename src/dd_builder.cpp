#include "dd_builder.h"

#include "logging.h"

using namespace planning_logic;

namespace dd_builder {

void construct_dd(dd_buildable &container, cnf_encoder &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Choosing correct dd building algorithm";

    if (options.linear) {
        if (options.timesteps >= 0) {
            construct_dd_linear(container, encoder, options);
        } else {
            construct_dd_without_timesteps(container, encoder, options);
        }
    } else if (options.layer) {
        construct_dd_by_layer_unidirectional(container, encoder, options);
    } else if (options.layer_bi) {
        construct_dd_by_layer_bidirectional(container, encoder, options);
    } else if (options.layer_expo) {
        construct_dd_by_layer_exponentially(container, encoder, options);
    } else {
        LOG_MESSAGE(log_level::error) << "No known dd building algorithm was choosen";
    }
    LOG_MESSAGE(log_level::info) << "Finished constructing final DD";
}

void construct_dd_linear(dd_buildable &container, cnf_encoder &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Building dd linear";
    LOG_MESSAGE(log_level::info) << "Ordering all clauses";
    std::vector<logic_primitive> all_primitives = conjoin_order::order_all_clauses(encoder, options);
    conjoin_primitives_linear(container, all_primitives, 0, false);
}

void construct_dd_by_layer_unidirectional(dd_buildable &container, cnf_encoder &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Building bdd by layers unidirectional";
    container.set_num_dds(3);

    std::string build_order = options.build_order;
    if (!conjoin_order::is_valid_layer_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Build order string is not compatible with layer building";
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
    const int layer_bdd_idx = 2;

    int permutation_direction, first_layer;
    if (options.reverse_layer_building) {
        permutation_direction = -1;
        first_layer = options.timesteps - 1;
    } else {
        permutation_direction = 1;
        first_layer = 0;
    }
    // permute variables by one timestep
    std::vector<int> permutation =
        encoder.m_symbol_map.calculate_permutation_by_timesteps(permutation_direction, options.timesteps);

    // build the initial layer bdds
    LOG_MESSAGE(log_level::info) << "Start building layer BDDs";
    // constructs the first layer
    LOG_MESSAGE(log_level::info) << "Building first layer";
    std::vector<logic_primitive> layer_primitives =
        conjoin_order::order_clauses_for_layer(encoder, layer_seed, first_layer);
    conjoin_primitives_linear(container, layer_primitives, layer_bdd_idx, true);
    container.reduce_heap();

    // TODO think about variable ordering
    // apply the extended variable map to the main bdd
    // single_step_bdd.set_variable_order(order_for_all_timesteps);
    // main_bdd.set_variable_order(order_for_all_timesteps);

    // build the bdd for the foundation bdd
    LOG_MESSAGE(log_level::info) << "Start building initial state foundation BDD";
    std::vector<logic_primitive> foundation_primitives =
        conjoin_order::order_clauses_for_foundation(encoder, ini_seed, options.timesteps);
    conjoin_primitives_linear(container, foundation_primitives, main_bdd_idx, true);

    // build the main bdd layer by layer
    int curr_layer = first_layer;
    int num_layer_conjoins = 0;
    while (num_layer_conjoins < options.timesteps) {
        LOG_MESSAGE(log_level::info) << "Adding timestep for t=" << curr_layer << " "
                                     << container.get_short_statistics(main_bdd_idx);
        container.conjoin_two_dds(main_bdd_idx, layer_bdd_idx, main_bdd_idx);

        // construct new bdd (only if this is not the last bdd)
        if (num_layer_conjoins != options.timesteps - 1) {
            if (options.use_layer_permutation) {
                container.permute_variables(permutation, layer_bdd_idx, layer_bdd_idx);
            } else {
                // build from scratch
                container.clear_dd(layer_bdd_idx);
                std::vector<logic_primitive> layer_primitives =
                    conjoin_order::order_clauses_for_layer(encoder, layer_seed, curr_layer + permutation_direction);
                conjoin_primitives_linear(container, layer_primitives, layer_bdd_idx, true);
            }
        }

        curr_layer += permutation_direction;
        num_layer_conjoins++;
    }
}

void construct_dd_by_layer_bidirectional(dd_buildable &container, cnf_encoder &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Building bdd by layers bidirectional";
    container.set_num_dds(3);

    std::string build_order = options.build_order;
    if (!conjoin_order::is_valid_layer_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Build order string is not compatible wit layer building";
        return;
    }

    // split the order into parts (in a really complicated manner)
    std::stringstream ss(options.build_order);
    std::string ini_seed, layer_seed, goal_seed;
    std::getline(ss, ini_seed, ':');
    std::getline(ss, layer_seed, ':');
    std::getline(ss, goal_seed, ':');

    const int main_begin_idx = 0;
    const int main_end_idx = options.share_foundations ? 0 : 1;
    const int layer_bdd_idx = 2;

    // permute variables by one timestep
    std::vector<int> forward_permutation =
        encoder.m_symbol_map.calculate_permutation_by_timesteps(1, options.timesteps);

    // build the initial layer bdds
    LOG_MESSAGE(log_level::info) << "Start building layer BDDs";
    // constructs the first layer
    const int first_layer = 0;
    LOG_MESSAGE(log_level::info) << "Building first layer";
    std::vector<logic_primitive> layer_primitives =
        conjoin_order::order_clauses_for_layer(encoder, layer_seed, first_layer);
    conjoin_primitives_linear(container, layer_primitives, layer_bdd_idx, true);
    container.reduce_heap();

    // apply the extended variable map to the main bdd
    // single_step_bdd.set_variable_order(order_for_all_timesteps);
    // main_bdd.set_variable_order(order_for_all_timesteps);

    // build the bdd for the foundations
    LOG_MESSAGE(log_level::info) << "Start building initial state foundation BDD";
    std::vector<logic_primitive> init_foundation_primitives =
        conjoin_order::order_clauses_for_foundation(encoder, ini_seed, options.timesteps);
    conjoin_primitives_linear(container, init_foundation_primitives, main_begin_idx, true);
    LOG_MESSAGE(log_level::info) << "Start building goal foundation BDD";
    std::vector<logic_primitive> goal_foundation_primitives =
        conjoin_order::order_clauses_for_foundation(encoder, goal_seed, options.timesteps);
    conjoin_primitives_linear(container, goal_foundation_primitives, main_end_idx, true);

    int t_begin = 0;
    int t_end = options.timesteps - 1;
    bool forward_step = true;

    while (t_begin <= t_end) {
        if (forward_step) {
            LOG_MESSAGE(log_level::info) << "Adding forward timestep for t=" << t_begin << " "
                                         << container.get_short_statistics(main_begin_idx);
            container.conjoin_two_dds(main_begin_idx, layer_bdd_idx, main_begin_idx);

            // we have to update the new bdd to conjoin
            // only if we need the next step
            if (t_end - t_begin > 1) {
                // construct new needed bdd
                if (options.use_layer_permutation) {
                    auto forward_perm =
                        encoder.m_symbol_map.calculate_permutation_by_timesteps(t_end - t_begin, options.timesteps);
                    container.permute_variables(forward_perm, layer_bdd_idx, layer_bdd_idx);
                } else {
                    container.clear_dd(layer_bdd_idx);
                    std::vector<logic_primitive> layer_primitives =
                        conjoin_order::order_clauses_for_layer(encoder, layer_seed, t_begin);
                    conjoin_primitives_linear(container, layer_primitives, layer_bdd_idx, true);
                }
            }
            t_begin++;
            forward_step = false;
        } else {
            LOG_MESSAGE(log_level::info) << "Adding backward timestep for t=" << t_end << " "
                                         << container.get_short_statistics(main_end_idx);
            container.conjoin_two_dds(main_end_idx, layer_bdd_idx, main_end_idx);

            // we have to update the new bdd to conjoin
            // only if we need the next step
            if (t_end - t_begin > 1) {
                // construct new needed bdd
                if (options.use_layer_permutation) {
                    auto backward_perm =
                        encoder.m_symbol_map.calculate_permutation_by_timesteps(t_begin - t_end, options.timesteps);
                    container.permute_variables(backward_perm, layer_bdd_idx, layer_bdd_idx);
                } else {
                    container.clear_dd(layer_bdd_idx);
                    std::vector<logic_primitive> layer_primitives =
                        conjoin_order::order_clauses_for_layer(encoder, layer_seed, t_end);
                    conjoin_primitives_linear(container, layer_primitives, layer_bdd_idx, true);
                }
            }
            t_end--;
            forward_step = true;
        }
    }

    if (!options.share_foundations) {
        LOG_MESSAGE(log_level::info) << "Doing final conjoin of foundations";
        container.conjoin_two_dds(main_begin_idx, main_end_idx, main_begin_idx);
    }
}

// unoptimized implementation of integer power
int ipow(int base, int exponent) {
    int result = 1;
    while (exponent != 0) {
        if ((exponent % 2) == 1) {
            result *= base;
            exponent -= 1;
        } else {
            base *= base;
            exponent /= 2;
        }
    }
    return result;
}

void construct_dd_by_layer_exponentially(dd_buildable &container, cnf_encoder &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Building bdd by layers exponentially";

    // TODO two ways: construct 2^ceil(log(t)) layers or construct the exact amount. See if first one is faster
    std::string build_order = options.build_order;
    if (!conjoin_order::is_valid_layer_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Build order string is not compatible wit layer building";
        return;
    }

    // split the order into parts (in a really complicated manner)
    std::stringstream ss(options.build_order);
    std::string ini_seed, layer_seed, goal_seed;
    std::getline(ss, ini_seed, ':');
    std::getline(ss, layer_seed, ':');
    std::getline(ss, goal_seed, ':');

    const int main_bdd_idx = 0;

    // build the bdd for the foundation bdd
    LOG_MESSAGE(log_level::info) << "Start building initial state foundation BDD";
    std::vector<logic_primitive> foundation_primitives =
        conjoin_order::order_clauses_for_foundation(encoder, ini_seed, options.timesteps);
    conjoin_primitives_linear(container, foundation_primitives, main_bdd_idx, true);

    // information about the block we want to build next
    int curr_block_size = 1;
    int curr_block_idx = 0;

    LOG_MESSAGE(log_level::info) << "Building first layer";
    std::vector<logic_primitive> layer_primitives =
        conjoin_order::order_clauses_for_layer(encoder, layer_seed, curr_block_idx);
    conjoin_primitives_linear(container, layer_primitives, curr_block_idx + 2, true);
    container.reduce_heap();
    curr_block_size *= 2;
    curr_block_idx += 1;

    while (curr_block_size < options.timesteps) {
        LOG_MESSAGE(log_level::info) << "Constructing " << curr_block_size << " layers";
        auto perm = encoder.m_symbol_map.calculate_permutation_by_timesteps(curr_block_size / 2, options.timesteps);
        container.permute_variables(perm, curr_block_idx + 1, curr_block_idx + 2);
        container.conjoin_two_dds(curr_block_idx + 1, curr_block_idx + 2, curr_block_idx + 2);

        curr_block_size *= 2;
        curr_block_idx += 1;
    }
    curr_block_idx--;

    LOG_MESSAGE(log_level::info) << "Finished exponential construction. Putting everything together";
    int total_size = 0;
    for (; curr_block_idx >= 0; curr_block_idx--) {
        int curr_block_size = ipow(2, curr_block_idx);

        if (total_size + curr_block_size <= options.timesteps) {
            LOG_MESSAGE(log_level::info) << "Adding block of size " << curr_block_size << " bdd has size "
                                         << total_size;

            auto perm = encoder.m_symbol_map.calculate_permutation_by_timesteps(total_size, options.timesteps);
            container.permute_variables(perm, curr_block_idx + 2, curr_block_idx + 2);
            container.conjoin_two_dds(main_bdd_idx, curr_block_idx + 2, main_bdd_idx);

            total_size += curr_block_size;

        } else {
            LOG_MESSAGE(log_level::info) << "Can't add block of size " << curr_block_size << " bdd has size "
                                         << total_size;
        }
        container.clear_dd(curr_block_idx + 2);
    }
}

bool goal_is_fullfilled(dd_buildable &container, cnf_encoder &encoder, int main_idx, int temp_idx, int timestep) {
    LOG_MESSAGE(log_level::info) << "Checking if goal is fulfilled in timestep " << timestep;
    std::vector<logic_primitive> goal_primitives = encoder.get_logic_primitives(goal, timestep);
    container.clear_dd(temp_idx);
    container.conjoin_two_dds(main_idx, temp_idx, temp_idx);
    conjoin_primitives_linear(container, goal_primitives, temp_idx, true);

    bool is_fulfilled = container.is_constant_false(temp_idx) ? false : true;
    container.clear_dd(temp_idx);

    LOG_MESSAGE(log_level::info) << "Goal is " << (is_fulfilled ? "" : "not") << " fulfilled";
    return is_fulfilled;
}

void construct_dd_without_timesteps(dd_buildable &container, cnf_encoder &encoder, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Building BDD without knowing the correct amount of timesteps";

    container.set_num_dds(2);

    // split the order into parts (in a really complicated manner)
    // default rympec::
    std::stringstream ss(options.build_order);
    std::string order;
    std::getline(ss, order, ':');

    // construct initial state
    std::vector<logic_primitive> temp = encoder.get_logic_primitives(ini_state, 0);
    conjoin_primitives_linear(container, temp, 0, true);

    int t = 0;
    while (true) {
        if (goal_is_fullfilled(container, encoder, 0, 1, t)) {
            // add the goal to the main bdd
            LOG_MESSAGE(log_level::info) << "Goal is fulfilled in layer " << t;
            temp = encoder.get_logic_primitives(goal, t);
            conjoin_primitives_linear(container, temp, 0, true);
            // conatiner.m_num_variables = encoder.m_symbol_map.get_num_variables();
            return;
        } else {
            // construct new bdd layer
            LOG_MESSAGE(log_level::info) << "Extending layer " << t;
            // TODO optimize this (i include mutex for variable twice)
            for(int i = 0; i < order.size(); i++){
                int curr_char = order[i];
                primitive_tag curr_tag = conjoin_order::char_tag_map[curr_char];
            }
            temp = conjoin_order::order_clauses_for_layer(encoder, order, t);
            conjoin_primitives_linear(container, temp, 0, true);
            t++;
        }
    }
}

void conjoin_primitives_linear(dd_buildable &dd, std::vector<logic_primitive> &logic_primitives, int dd_index,
                               bool silent) {
    if (!silent) {
        LOG_MESSAGE(log_level::info) << "Start constructing DD by lineary adding logic primitives";
    }
    // conjoin the clauses in the correct order
    int percent = 0;
    for (int i = 0; i < logic_primitives.size(); i++) {
        planning_logic::logic_primitive primitive = logic_primitives[i];

        switch (primitive.m_type) {
            case logic_clause:
                dd.add_clause_primitive(primitive.m_data, dd_index);
                break;
            case logic_dnf:
                dd.add_dnf_primitive(primitive.m_dnf_data, dd_index);
                break;
            case logic_eo:
                dd.add_exactly_one_primitive(primitive.m_data, dd_index);
                break;
            default:
                LOG_MESSAGE(log_level::warning) << "Unknown logic primitive type during DD construction";
                break;
        }
        int new_percent = (100 * (i + 1)) / logic_primitives.size();
        if (new_percent > percent) {
            percent = new_percent;
            if (!silent) {
                LOG_MESSAGE(log_level::info)
                    << "Conjoined " << percent << "% of all clauses. " + dd.get_short_statistics(dd_index);
            }
        }
    }
    if (!silent) {
        LOG_MESSAGE(log_level::info) << "Finished constructing DD";
    }
}

}  // namespace dd_builder