#include "dd_builder_variable_order.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <set>

#include "dd_builder_conjoin_order.h"
#include "force.h"
#include "logging.h"

using namespace planning_logic;
using namespace encoder;

namespace variable_order {

// used to interpret the order of clauses from the command line options
std::map<char, std::vector<variable_tag>> char_tag_map = {
    {'v', {variable_plan_var, variable_plan_binary_var}},
    {'o', {variable_plan_op, variable_plan_binary_op}},
    {'h', {variable_h_amost_variable, variable_h_amost_operator, variable_h_amost_mutex}},
};

bool is_valid_variable_order_string(std::string variable_order) {
    // check if string is permutation
    std::string standart_permutation = "vox:h";
    if (!std::is_permutation(variable_order.begin(), variable_order.end(), standart_permutation.begin(),
                             standart_permutation.end())) {
        LOG_MESSAGE(log_level::error) << "Variable order has to be a permutation of " + standart_permutation +
                                             " but is "
                                      << variable_order;
        return false;
    }
    return true;
}

categorized_variables categorize_variables(planning_logic::plan_to_cnf_map &symbol_map, int timesteps) {
    LOG_MESSAGE(log_level::info) << "Starting to categorize variables";

    categorized_variables tagged_variables;

    // give every vector #timesteps+1 buckets
    for (int tag_int = variable_plan_var; tag_int <= variable_none; tag_int++) {
        variable_tag t = static_cast<variable_tag>(tag_int);
        tagged_variables[t] = std::vector<std::vector<int>>(timesteps + 1);
    }

    // iterate over every variable of the cnf problem
    for (std::map<tagged_variable, int>::iterator iter = symbol_map.m_variable_map.begin();
         iter != symbol_map.m_variable_map.end(); iter++) {
        // get information about the variable
        tagged_variable tag_var = iter->first;
        int cnf_index = iter->second;
        variable_tag tag = std::get<0>(tag_var);
        int timestep = std::get<1>(tag_var);
        tagged_variables[tag][timestep].push_back(cnf_index);
    }

    // print info about how many variables each tag has
    for (int tag_int = variable_plan_var; tag_int <= variable_none; tag_int++) {
        variable_tag t = static_cast<variable_tag>(tag_int);

        int total_variables = 0;
        for (int k = 0; k <= timesteps; k++) {
            total_variables += tagged_variables[t][k].size();
        }
        LOG_MESSAGE(log_level::info) << "Categorized " << total_variables << " variables of tag " << t;
    }

    return tagged_variables;
}

// current_order[i] = what variable is at layer i?
std::vector<int> put_variables_of_tag_first(encoder_abstract &encoder, std::vector<int> &current_order,
                                            primitive_tag front_tag, int timesteps) {
    // find all variables that are affected by the given tag
    std::set<int> front_variables;
    for (int t = 0; t <= timesteps; t++) {
        for (logic_primitive c : encoder.get_logic_primitives(front_tag, t)) {
            for (int j = 0; j < c.m_data.size(); j++) {
                front_variables.insert(std::abs(c.m_data[j]));
            }
        }
    }

    // first append all the goal variables and than the rest (the rest keeps its order)
    std::vector<int> new_order;
    for (int i : front_variables) {
        new_order.push_back(i);
    }
    for (int i = 0; i < current_order.size(); i++) {
        if (front_variables.find(current_order[i]) == front_variables.end()) {
            new_order.push_back(current_order[i]);
        }
    }

    return new_order;
}

std::vector<int> order_variables(encoder_abstract &encoder, option_values &options) {
    std::string variable_order = options.variable_order;
    if (!is_valid_variable_order_string(variable_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following variable order " << variable_order;
        return std::vector<int>();
    }

    std::vector<int> ordered_variables;

    if (options.var_order_force) {
        std::vector<std::tuple<int, int>> temp = create_force_var_order_mapping(encoder, options);
        for (int i = 0; i < temp.size(); i++) {
            ordered_variables.push_back(std::get<0>(temp[i]));
        }
    } else if (options.var_order_custom) {
        std::vector<std::tuple<int, int>> temp = create_custom_var_order_mapping(encoder, options);
        for (int i = 0; i < temp.size(); i++) {
            ordered_variables.push_back(std::get<0>(temp[i]));
        }
    } else if (options.var_order_custom_force) {
        std::vector<std::tuple<int, int, int>> temp = create_custom_force_var_order_mapping(encoder, options);
        for (int i = 0; i < temp.size(); i++) {
            ordered_variables.push_back(std::get<0>(temp[i]));
        }
    } else {
        LOG_MESSAGE(log_level::error) << "No known variable order selected";
    }

    // move goal or initial_state variable first
    if (options.goal_variables_first) {
        ordered_variables = put_variables_of_tag_first(encoder, ordered_variables, goal, options.timesteps);
    }
    if (options.initial_state_variables_first) {
        ordered_variables = put_variables_of_tag_first(encoder, ordered_variables, ini_state, options.timesteps);
    }

    LOG_MESSAGE(log_level::info) << "Sorted a total of " << ordered_variables.size() << " variables";
    return ordered_variables;
}

std::vector<std::tuple<int, int>> create_custom_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                  option_values &options) {
    LOG_MESSAGE(log_level::info) << "Calculating custom variable order";

    // calculate order given by order string
    int custom_order_counter;  // counter that implies the partial order
    std::vector<std::tuple<int, int>> result;
    result.push_back(std::make_tuple(0, custom_order_counter));  // o variable at first position
    custom_order_counter++;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(options.variable_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // categorize the variables
    categorized_variables tagged_variables = categorize_variables(encoder.m_symbol_map, options.timesteps);

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];

        // add the interleved part
        if (current_char == 'x') {
            // sort the interleaved part
            for (int t = 0; t <= options.timesteps; t++) {
                for (int j = 0; j < interleaved_order.size(); j++) {
                    char interleaved_char = interleaved_order[j];
                    for (int vt = 0; vt < char_tag_map[interleaved_char].size(); vt++) {
                        variable_tag order_tag = char_tag_map[interleaved_char][vt];
                        // add all the variables for timestep t and tag order_tag
                        for (int k = 0; k < tagged_variables[order_tag][t].size(); k++) {
                            result.push_back(std::make_tuple(tagged_variables[order_tag][t][k], custom_order_counter));
                        }
                    }
                }
                custom_order_counter++;  // increase counter after every timestep
            }
            continue;
        }
        // add all timesteps for the corresponding var tag
        for (int vt = 0; vt < char_tag_map[current_char].size(); vt++) {
            variable_tag order_tag = char_tag_map[current_char][vt];
            for (int t = 0; t <= options.timesteps; t++) {
                for (int k = 0; k < tagged_variables[order_tag][t].size(); k++) {
                    result.push_back(std::make_tuple(tagged_variables[order_tag][t][k], custom_order_counter));
                }
            }
            custom_order_counter++;  // increase counter for every group
        }
    }
    return result;
}

// returns a list of var_index, order_index tuples
std::vector<std::tuple<int, int>> create_force_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                 option_values &options) {
    LOG_MESSAGE(log_level::info) << "Calculating force variable order";

    // collect all logic primitives of the planning problem
    LOG_MESSAGE(log_level::info) << "Collecting primitives";
    std::vector<std::tuple<logic_primitive, int>> all_primitives =
        conjoin_order::create_custom_clause_order_mapping(encoder, options);
    std::vector<logic_primitive> stripped_primitives;
    for (int i = 0; i < all_primitives.size(); i++) {
        stripped_primitives.push_back(std::get<0>(all_primitives[i]));
    }

    // calculate initial variable order (use identity)
    std::vector<int> initial_order(encoder.m_symbol_map.get_num_variables() + 1);  // + one because of zero variable
    for (int i = 0; i < initial_order.size(); i++) {
        initial_order[i] = i;
    }

    // enable for random initial permutation
    if (options.force_random_seed) {
        auto rng = std::default_random_engine{};
        std::shuffle(std::begin(initial_order), std::end(initial_order), rng);
    }

    // calculate force order
    LOG_MESSAGE(log_level::info) << "Apllying force algorithm";
    std::vector<int> force_order = force_variable_order(initial_order, stripped_primitives);

    LOG_MESSAGE(log_level::info) << "Transforming to result";
    std::vector<std::tuple<int, int>> result;
    for (int i = 0; i < force_order.size(); i++) {
        result.push_back(std::make_tuple(force_order[i], i));
    }

    return result;
}

std::vector<std::tuple<int, int, int>> create_custom_force_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                             option_values &options) {
    LOG_MESSAGE(log_level::info) << "Calculating custom force variable order";

    std::vector<std::tuple<int, int>> custom_order = create_custom_var_order_mapping(encoder, options);
    std::vector<std::tuple<int, int>> force_order = create_force_var_order_mapping(encoder, options);

    // combine tuples to triples
    std::vector<std::tuple<int, int, int>> combined_order;
    std::map<int, int> var_order_map;  // helper map, maps var index to custom order
    for (int i = 0; i < custom_order.size(); i++) {
        var_order_map[std::get<0>(custom_order[i])] = std::get<1>(custom_order[i]);
    }
    for (int i = 0; i < force_order.size(); i++) {
        int var_idx = std::get<0>(force_order[i]);
        int force_idx = std::get<1>(force_order[i]);
        combined_order.push_back(std::make_tuple(var_idx, var_order_map[var_idx], force_idx));
    }

    // sort by custom order and use force order as tiebreaker
    sort(combined_order.begin(), combined_order.end(),
         [](const std::tuple<int, int, int> &lhs, const std::tuple<int, int, int> &rhs) {
             if (std::get<1>(lhs) == std::get<1>(rhs)) {
                 return std::get<2>(lhs) < std::get<2>(rhs);
             } else {
                 return std::get<1>(lhs) < std::get<1>(rhs);
             }
         });

    return combined_order;
}

};  // namespace variable_order