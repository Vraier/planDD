#include "dd_builder_conjoin_order.h"

#include <algorithm>
#include <random>

#include "bottom_up.h"
#include "dd_builder_variable_order.h"
#include "force.h"
#include "logging.h"

using namespace planning_logic;
using namespace encoder;

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

std::vector<logic_primitive> order_all_clauses(encoder_abstract &encoder, option_values &options) {
    std::string build_order = options.build_order;
    if (!is_valid_conjoin_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following conjoin order " << build_order;
        return std::vector<logic_primitive>();
    }

    std::vector<logic_primitive> ordered_primitives;

    if (options.clause_order_force) {
        std::vector<std::tuple<logic_primitive, int>> temp = create_force_clause_order_mapping(encoder, options);
        for (int i = 0; i < temp.size(); i++) {
            ordered_primitives.push_back(std::get<0>(temp[i]));
        }
    } else if (options.clause_order_custom) {
        std::vector<std::tuple<logic_primitive, int>> temp = create_custom_clause_order_mapping(encoder, options);
        for (int i = 0; i < temp.size(); i++) {
            ordered_primitives.push_back(std::get<0>(temp[i]));
        }
    } else if (options.clause_order_custom_force) {
        std::vector<std::tuple<logic_primitive, int>> order = create_custom_clause_order_mapping(encoder, options);
        std::vector<std::tuple<logic_primitive, int>> tiebreak = create_force_clause_order_mapping(encoder, options);
        std::vector<std::tuple<logic_primitive, int, int>> temp = create_mixed_clause_order_mapping(order, tiebreak);
        for (int i = 0; i < temp.size(); i++) {
            ordered_primitives.push_back(std::get<0>(temp[i]));
        }
    } else if (options.clause_order_bottom_up) {
        std::vector<std::tuple<logic_primitive, int>> temp = create_bottom_up_clause_order_mapping(encoder, options);
        for (int i = 0; i < temp.size(); i++) {
            ordered_primitives.push_back(std::get<0>(temp[i]));
        }
    } else if (options.clause_order_custom_bottom_up) {
        std::vector<std::tuple<logic_primitive, int>> order = create_custom_clause_order_mapping(encoder, options);
        std::vector<std::tuple<logic_primitive, int>> tiebreak =
            create_bottom_up_clause_order_mapping(encoder, options);
        std::vector<std::tuple<logic_primitive, int, int>> temp = create_mixed_clause_order_mapping(order, tiebreak);
        for (int i = 0; i < temp.size(); i++) {
            ordered_primitives.push_back(std::get<0>(temp[i]));
        }
    } else {
        LOG_MESSAGE(log_level::error) << "No known conjoin order selected";
    }

    // this reverses the order of the clauses. It allows the variables with the highes timesteps to be conjoined first
    if (options.reverse_order) {
        LOG_MESSAGE(log_level::info) << "Reversing order of the logic primitives";
        std::reverse(ordered_primitives.begin(), ordered_primitives.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << ordered_primitives.size() << " primitives";
    return ordered_primitives;
}

std::vector<std::tuple<logic_primitive, int>> create_custom_clause_order_mapping(encoder_abstract &encoder,
                                                                                 option_values &options) {
    LOG_MESSAGE(log_level::info) << "Calculating custom conjoin order";

    std::string build_order = options.build_order;
    int custom_order_counter;  // counter that implies the partial order

    // contains the result at the end
    std::vector<std::tuple<logic_primitive, int>> result;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order, tail_part;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');
    std::getline(ss, tail_part, ':');

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];
        // add the interleved part
        if (current_char == 'x') {
            // sort the interleaved part
            for (int t = 0; t <= options.timesteps; t++) {
                for (int j = 0; j < interleaved_order.size(); j++) {
                    char interleaved_char = interleaved_order[j];
                    primitive_tag order_tag = char_tag_map[interleaved_char];
                    // only add last timestep for exact one var clauses
                    if (t != options.timesteps || interleaved_char == 'r') {
                        std::vector<logic_primitive> temp_clauses =
                            collect_primitives_for_single_timestep(encoder, order_tag, t);
                        for (int k = 0; k < temp_clauses.size(); k++) {
                            result.push_back(std::make_tuple(temp_clauses[k], custom_order_counter));
                        }
                    }
                    if(options.split_inside_timestep){
                        custom_order_counter++;  // increase counter after every category in a timestep
                    }
                }
                custom_order_counter++;  // increase counter after every timestep
            }
            // sort the disjoint part
        } else {
            primitive_tag order_tag = char_tag_map[current_char];
            std::vector<logic_primitive> temp_clauses;
            if (order_tag == ini_state) {
                temp_clauses = collect_primitives_for_single_timestep(encoder, order_tag, 0);
            } else if (order_tag == goal) {
                temp_clauses = collect_primitives_for_single_timestep(encoder, order_tag, options.timesteps);
            } else if (order_tag == eo_var) {
                temp_clauses = collect_primitives_for_all_timesteps(encoder, order_tag, options.timesteps);
            } else {  // eo_clause?
                temp_clauses = collect_primitives_for_all_timesteps(encoder, order_tag, options.timesteps - 1);
            }
            for (int k = 0; k < temp_clauses.size(); k++) {
                result.push_back(std::make_tuple(temp_clauses[k], custom_order_counter));
            }
            custom_order_counter++;  // increase counter for every group
        }
    }
    return result;
}

std::vector<std::tuple<logic_primitive, int>> create_force_clause_order_mapping(encoder::encoder_abstract &encoder,
                                                                                option_values &options) {
    LOG_MESSAGE(log_level::info) << "Calculating force conjoin order";

    // order the primitives by custom order
    std::vector<std::tuple<logic_primitive, int>> all_primitives =
        conjoin_order::create_custom_clause_order_mapping(encoder, options);
    std::vector<logic_primitive> stripped_primitives;
    for (int i = 0; i < all_primitives.size(); i++) {
        stripped_primitives.push_back(std::get<0>(all_primitives[i]));
    }

    // use identity for initial mapping
    std::vector<int> initial_mapping(stripped_primitives.size());
    for (int i = 0; i < stripped_primitives.size(); i++) {
        initial_mapping[i] = i;
    }
    // enable for random initial permutation
    if (options.force_random_seed) {
        auto rng = std::default_random_engine{};
        std::shuffle(std::begin(initial_mapping), std::end(initial_mapping), rng);
    }

    std::vector<int> force_order = variable_order::force_clause_order(initial_mapping, stripped_primitives,
                                                                      encoder.m_symbol_map.get_num_variables() + 1);

    std::vector<std::tuple<logic_primitive, int>> result;
    for (int i = 0; i < force_order.size(); i++) {
        result.push_back(std::make_tuple(stripped_primitives[force_order[i]], i));
    }

    return result;
}

std::vector<std::tuple<logic_primitive, int>> create_bottom_up_clause_order_mapping(encoder::encoder_abstract &encoder,
                                                                                    option_values &options) {
    LOG_MESSAGE(log_level::info) << "Calculating bottom up conjoin order";
    
    std::vector<std::tuple<logic_primitive, int>> custom_order = create_custom_clause_order_mapping(encoder, options);
    std::vector<logic_primitive> bottom_up_order;
    for (int i = 0; i < custom_order.size(); i++) {
        bottom_up_order.push_back(std::get<0>(custom_order[i]));
    }

    std::vector<int> pos_to_var = variable_order::order_variables(encoder, options);
    std::vector<int> var_to_pos(pos_to_var.size());
    for (int i = 0; i < pos_to_var.size(); i++) {
        var_to_pos[pos_to_var[i]] = i;
    }

    sort_bottom_up(bottom_up_order, 0, bottom_up_order.size(), var_to_pos);

    std::vector<std::tuple<logic_primitive, int>> result;
    for (int i = 0; i < bottom_up_order.size(); i++) {
        result.push_back(std::make_tuple(bottom_up_order[i], i));
    }
    return result;
}

std::vector<std::tuple<logic_primitive, int, int>> create_mixed_clause_order_mapping(
    std::vector<std::tuple<planning_logic::logic_primitive, int>> &order,
    std::vector<std::tuple<planning_logic::logic_primitive, int>> &tiebreaker) {
    LOG_MESSAGE(log_level::info) << "Calculating mixed conjoin order";

    // combine tuples to triples
    std::vector<std::tuple<logic_primitive, int, int>> combined_order;
    std::map<logic_primitive, int> clause_order_map;  // helper map, maps var index to custom order
    for (int i = 0; i < order.size(); i++) {
        clause_order_map[std::get<0>(order[i])] = std::get<1>(order[i]);
    }
    for (int i = 0; i < tiebreaker.size(); i++) {
        logic_primitive force_primitive = std::get<0>(tiebreaker[i]);
        int force_idx = std::get<1>(tiebreaker[i]);
        combined_order.push_back(std::make_tuple(force_primitive, clause_order_map[force_primitive], force_idx));
    }

    // sort by custom order and use force order as tiebreaker
    sort(combined_order.begin(), combined_order.end(),
         [](const std::tuple<logic_primitive, int, int> &lhs, const std::tuple<logic_primitive, int, int> &rhs) {
             if (std::get<1>(lhs) == std::get<1>(rhs)) {
                 return std::get<2>(lhs) < std::get<2>(rhs);
             } else {
                 return std::get<1>(lhs) < std::get<1>(rhs);
             }
         });

    return combined_order;
}

std::vector<logic_primitive> order_clauses_for_layer(encoder_abstract &encoder, std::string &order_string, int layer) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for single layer " << layer;

    // contains the result at the end
    std::vector<logic_primitive> result_clauses;
    std::vector<logic_primitive> temp_clauses;

    for (char c : order_string) {
        primitive_tag order_tag = char_tag_map[c];

        temp_clauses = collect_primitives_for_single_timestep(encoder, order_tag, layer);
        // if we add at least or at most one var clauses, also do it for the second timestep
        if (c == 'r' || c == 'm') {
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

std::vector<logic_primitive> order_clauses_for_foundation(encoder_abstract &encoder, std::string &order_string,
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

std::vector<logic_primitive> collect_primitives_for_all_timesteps(encoder_abstract &encoder,
                                                                  primitive_tag primitive_type, int timesteps) {
    std::vector<logic_primitive> result_primitives;

    for (int t = 0; t <= timesteps; t++) {
        std::vector<logic_primitive> single_timestep = encoder.get_logic_primitives(primitive_type, t);
        result_primitives.insert(result_primitives.end(), single_timestep.begin(), single_timestep.end());
    }
    return result_primitives;
}

std::vector<logic_primitive> collect_primitives_for_single_timestep(encoder_abstract &encoder,
                                                                    primitive_tag primitive_type, int timestep) {
    return encoder.get_logic_primitives(primitive_type, timestep);
}

};  // namespace conjoin_order