#include "dd_builder_conjoin_order.h"

#include "logging.h"

#include <algorithm>

using namespace planning_logic;

namespace conjoin_order {

// used to interpret the order of clauses from the command line options
std::map<char, primitive_tag> char_tag_map = {
    {'i', ini_state}, {'g', goal},   {'r', eo_var}, {'y', eo_op},
    {'m', mutex},     {'p', precon}, {'e', effect}, {'c', frame},
};

bool is_valid_conjoin_order_string(std::string &build_order) {
    // check if string is permutation
    std::string standart_permutation = "igrymx:pec:";
    if (!std::is_permutation(build_order.begin(), build_order.end(), standart_permutation.begin(),
                             standart_permutation.end())) {
        LOG_MESSAGE(log_level::error) << "Build order has to be a permutation of " + standart_permutation + " but is "
                                      << build_order;
        return false;
    }

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order, tail_part;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');
    std::getline(ss, tail_part, ':');

    // check if first part contains x, i and g
    if (disjoin_order.find("x") == std::string::npos || disjoin_order.find("i") == std::string::npos ||
        disjoin_order.find("g") == std::string::npos) {
        LOG_MESSAGE(log_level::error) << "First part of order has to contain i, g and x but is " << disjoin_order;
        return false;
    }

    return true;
}

int count_char(std::string str, char c) {
    int count = 0;
    for (char a : str) {
        if (a == c) {
            count++;
        }
    }
    return count;
}

bool is_valid_layer_order_string(std::string &build_order) {
    // check if string is permutation
    int num_colons = count_char(build_order, ':');
    if (num_colons != 2) {
        LOG_MESSAGE(log_level::error) << "Build contains wrong number of colons (must be 2)";
        return false;
    }
    return true;
}

std::vector<logic_primitive> order_all_clauses(cnf_encoder &encoder, option_values &options) {
    std::string build_order = options.build_order;

    if (!is_valid_conjoin_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following conjoin order " << build_order;
        return std::vector<logic_primitive>();
    }

    // contains the result at the end
    std::vector<logic_primitive> interleaved_primitives;
    std::vector<logic_primitive> total_primitives;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order, tail_part;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');
    std::getline(ss, tail_part, ':');

    // sort the interleaved part
    for (int t = 0; t <= options.timesteps; t++) {
        for (int i = 0; i < interleaved_order.size(); i++) {
            char current_char = interleaved_order[i];
            primitive_tag order_tag = char_tag_map[current_char];

            // only add last timestep for exact one var clauses
            if (t != options.timesteps || current_char == 'r') {
                std::vector<logic_primitive> temp_clauses =
                    collect_primitives_for_single_timestep(encoder, order_tag, t);
                interleaved_primitives.insert(interleaved_primitives.end(), temp_clauses.begin(), temp_clauses.end());
            }
        }
    }

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];
        // add the interleved part
        if (current_char == 'x') {
            total_primitives.insert(total_primitives.end(), interleaved_primitives.begin(),
                                    interleaved_primitives.end());
        } else {
            primitive_tag order_tag = char_tag_map[current_char];
            std::vector<logic_primitive> temp_clauses;
            if (order_tag == ini_state) {
                temp_clauses = collect_primitives_for_single_timestep(encoder, order_tag, 0);
            } else if (order_tag == goal) {
                temp_clauses = collect_primitives_for_single_timestep(encoder, order_tag, options.timesteps);
            } else if (order_tag == eo_var) {
                temp_clauses = collect_primitives_for_all_timesteps(encoder, order_tag, options.timesteps);
            } else {
                temp_clauses = collect_primitives_for_all_timesteps(encoder, order_tag, options.timesteps - 1);
            }
            total_primitives.insert(total_primitives.end(), temp_clauses.begin(), temp_clauses.end());
        }
    }

    // this reverses the order of the clauses. It allows the variables with the highes timesteps to be conjoined first
    if (options.reverse_order) {
        LOG_MESSAGE(log_level::info) << "Reversing order of the logic primitives";
        std::reverse(total_primitives.begin(), total_primitives.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << total_primitives.size() << " primitives";
    return total_primitives;
}

std::vector<logic_primitive> order_clauses_for_layer(cnf_encoder &encoder, std::string &order_string, int layer) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for single layer " << layer;

    // contains the result at the end
    std::vector<logic_primitive> result_clauses;
    std::vector<logic_primitive> temp_clauses;

    for (char c : order_string) {
        primitive_tag order_tag = char_tag_map[c];

        temp_clauses = collect_primitives_for_single_timestep(encoder, order_tag, layer);
        // if we add at least or at most one var clauses, also do it for the second timestep
        if (c == 'r' || c == 'y' || c == 'm') {
            // does NOT conatin pec
            std::vector<logic_primitive> second_layer =
                collect_primitives_for_single_timestep(encoder, order_tag, layer + 1);
            temp_clauses.insert(temp_clauses.end(), second_layer.begin(), second_layer.end());
        }
        result_clauses.insert(result_clauses.end(), temp_clauses.begin(), temp_clauses.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << result_clauses.size() << " clauses for single layer "
                                 << layer;
    return result_clauses;
}

std::vector<logic_primitive> order_clauses_for_foundation(cnf_encoder &encoder, std::string &order_string,
                                                          int timesteps) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for foundation";

    // contains the result at the end
    std::vector<logic_primitive> result_clauses;
    std::vector<logic_primitive> temp_clauses;

    for (char c : order_string) {
        primitive_tag order_tag = char_tag_map[c];
        temp_clauses = collect_primitives_for_all_timesteps(encoder, order_tag, timesteps);
        result_clauses.insert(result_clauses.end(), temp_clauses.begin(), temp_clauses.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << result_clauses.size() << " clauses for foundation";
    return result_clauses;
}

std::vector<logic_primitive> collect_primitives_for_all_timesteps(cnf_encoder &encoder, primitive_tag primitive_type,
                                                                  int timesteps) {
    std::vector<logic_primitive> result_primitives;

    for (int t = 0; t <= timesteps; t++) {
        std::vector<logic_primitive> single_timestep = encoder.get_logic_primitives(primitive_type, t);
        result_primitives.insert(result_primitives.end(), single_timestep.begin(), single_timestep.end());
    }
    return result_primitives;
}

std::vector<logic_primitive> collect_primitives_for_single_timestep(cnf_encoder &encoder, primitive_tag primitive_type,
                                                                    int timestep) {
    return encoder.get_logic_primitives(primitive_type, timestep);
}

};  // namespace conjoin_order