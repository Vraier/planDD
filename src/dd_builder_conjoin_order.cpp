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
    std::string standart_permutation = "grtyumix:pec:";
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
    std::vector<logic_primitive> interleved_clauses;
    std::vector<logic_primitive> total_clauses;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // sort the interleaved part
    for (int t = 0; t <= options.timesteps; t++) {
        for (int i = 0; i < interleaved_order.size(); i++) {
            char current_char = interleaved_order[i];

            std::vector<logic_primitive> temp_clauses =
                collect_primitives_for_single_timestep(encoder, current_char, t);
            interleved_clauses.insert(interleved_clauses.end(), temp_clauses.begin(), temp_clauses.end());
        }
    }

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];
        // add the interleved part
        if (current_char == 'x') {
            total_clauses.insert(total_clauses.end(), interleved_clauses.begin(), interleved_clauses.end());
            continue;
        }

        std::vector<logic_primitive> temp_clauses = collect_primitives_for_all_timesteps(encoder, current_char);
        total_clauses.insert(total_clauses.end(), temp_clauses.begin(), temp_clauses.end());
    }

    // this reverses the order of the clauses. It allows the variables with the highes timesteps to be conjoined first
    if (options.reverse_order) {
        LOG_MESSAGE(log_level::info) << "Reversing order of the logic primitives";
        std::reverse(total_clauses.begin(), total_clauses.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << total_clauses.size() << " clauses";
    return total_clauses;
}

std::vector<tagged_logic_primitiv> order_clauses_for_layer(planning_logic::formula &cnf, int layer,
                                                           std::string &order_string) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for single layer " << layer;

    // contains the result at the end
    std::vector<tagged_logic_primitiv> result_clauses;
    std::vector<tagged_logic_primitiv> temp_clauses;

    for (char c : order_string) {
        temp_clauses = collect_primitives_for_single_timestep(cnf, c, layer);
        // if we add at least or at most one var clauses, also do it for the second timestep
        if (c == 'r' || c == 't' || c == 'y' || c == 'u' || c == 'm') {
            // does NOT conatin pec
            std::vector<tagged_logic_primitiv> second_layer = collect_primitives_for_single_timestep(cnf, c, layer + 1);
            temp_clauses.insert(temp_clauses.end(), second_layer.begin(), second_layer.end());
        }
        result_clauses.insert(result_clauses.end(), temp_clauses.begin(), temp_clauses.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << result_clauses.size() << " clauses for single layer "
                                 << layer;
    return result_clauses;
}

std::vector<tagged_logic_primitiv> order_clauses_for_foundation(planning_logic::formula &cnf,
                                                                std::string &order_string) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for foundation";

    // contains the result at the end
    std::vector<tagged_logic_primitiv> result_clauses;
    std::vector<tagged_logic_primitiv> temp_clauses;

    for (char c : order_string) {
        temp_clauses = collect_primitives_for_all_timesteps(cnf, c);
        result_clauses.insert(result_clauses.end(), temp_clauses.begin(), temp_clauses.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << result_clauses.size() << " clauses for foundation";
    return result_clauses;
}

std::vector<logic_primitive> collect_primitives_for_all_timesteps(cnf_encoder &encoder, char primitive_type,
                                                                  int timesteps) {
    std::vector<logic_primitive> result_primitives;
    primitive_tag order_tag = char_tag_map[primitive_type];

    for (int t = 0; t <= timesteps; t++) {
        std::vector<logic_primitive> single_timestep = encoder.get_logic_primitives(order_tag, t);
        result_primitives.insert(result_primitives.end(), single_timestep.begin(), single_timestep.end());
    }
    return result_primitives;
}

std::vector<logic_primitive> collect_primitives_for_single_timestep(cnf_encoder &encoder, char primitive_type,
                                                                    int timestep) {
    primitive_tag order_tag = char_tag_map[primitive_type];
    return encoder.get_logic_primitives(order_tag, timestep);
}

};  // namespace conjoin_order