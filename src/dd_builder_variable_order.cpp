#include "dd_builder_variable_order.h"

#include <iostream>
#include <set>

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

bool is_valid_variable_order_string(std::string build_order) {
    // check if string is permutation
    std::string standart_permutation = "vox:h";
    if (!std::is_permutation(build_order.begin(), build_order.end(), standart_permutation.begin(),
                             standart_permutation.end())) {
        LOG_MESSAGE(log_level::error) << "Variable order has to be a permutation of " + standart_permutation +
                                             " but is "
                                      << build_order;
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
    std::string build_order = options.variable_order;

    if (!is_valid_variable_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following variable order " << build_order;
        return std::vector<int>();
    }

    // categorize the variables
    categorized_variables tagged_variables = categorize_variables(encoder.m_symbol_map, options.timesteps);

    // contains the result at the end
    std::vector<int> interleved_variables;
    std::vector<int> total_variables;
    total_variables.push_back(0);  // the 0th variable does not exist

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // sort the interleaved part
    for (int t = 0; t <= options.timesteps; t++) {
        for (int i = 0; i < interleaved_order.size(); i++) {
            // LOG_MESSAGE(log_level::trace) << "Sorting variable tag " << interleaved_order[i] << " for timestep " <<
            // t;
            char current_char = interleaved_order[i];
            for (int vt = 0; vt < char_tag_map[current_char].size(); vt++) {
                variable_tag order_tag = char_tag_map[current_char][vt];
                // add all the variables for timestep t and tag order_tag
                interleved_variables.insert(interleved_variables.end(), tagged_variables[order_tag][t].begin(),
                                            tagged_variables[order_tag][t].end());
            }
        }
    }

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];
        // LOG_MESSAGE(log_level::trace) << "Sorting variable tag " << disjoin_order[i] << " for every timestep";

        // add the interleved part
        if (current_char == 'x') {
            total_variables.insert(total_variables.end(), interleved_variables.begin(), interleved_variables.end());
            continue;
        }
        // add all timesteps for the corresponding var tag
        for (int vt = 0; vt < char_tag_map[current_char].size(); vt++) {
            variable_tag order_tag = char_tag_map[current_char][vt];
            for (int t = 0; t <= options.timesteps; t++) {
                total_variables.insert(total_variables.end(), tagged_variables[order_tag][t].begin(),
                                       tagged_variables[order_tag][t].end());
            }
        }
    }

    // move goal or initial_state variable first
    if (options.goal_variables_first) {
        total_variables = put_variables_of_tag_first(encoder, total_variables, goal, options.timesteps);
    }
    if (options.initial_state_variables_first) {
        total_variables = put_variables_of_tag_first(encoder, total_variables, ini_state, options.timesteps);
    }

    LOG_MESSAGE(log_level::info) << "Sorted a total of " << total_variables.size() << " variables";
    return total_variables;
}

// returns a list of var_index, order_index tuples
std::vector<std::tuple<int, int>> create_force_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                 option_values &options) {

    LOG_MESSAGE(log_level::info) << "Calculating force variable order";

    // collect all logic primitives of the planning problem
    std::vector<planning_logic::logic_primitive> all_primitives;
    for (int t = 0; t <= options.timesteps; t++) {
        for (int typ = ini_state; typ < planning_logic::none; typ++) {
            std::vector<planning_logic::logic_primitive> temp =
                encoder.get_logic_primitives(static_cast<planning_logic::primitive_tag>(typ), t);
            all_primitives.insert(all_primitives.end(), temp.begin(), temp.end());
        }
    }

    // calculate initial variable order (use identity)
    std::vector<int> initial_order(encoder.m_symbol_map.get_num_variables());
    for (int i = 0; i < initial_order.size(); i++) {
        initial_order[i] = i;
    }

    // calculate force order
    std::vector<int> force_order = force_variable_order(initial_order, all_primitives);

    std::vector<std::tuple<int, int>> result;
    for (int i = 0; i < force_order.size(); i++) {
        result.push_back(std::make_tuple(force_order[i], i));
    }

    return result;
}

std::vector<std::tuple<int, int>> create_custom_var_order_mapping(encoder::encoder_abstract &encoder,
                                                                 option_values &options) {

    LOG_MESSAGE(log_level::info) << "Calculating custom variable order";
    
    // calculate order given by order string
    int custom_order_counter;  // counter that implies the partial order
    std::vector<std::tuple<int, int>> result;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(options.build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // categorize the variables
    categorized_variables tagged_variables = categorize_variables(encoder.m_symbol_map, options.timesteps);

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];
        // add the interleved part
        if (current_char == 'x') {
            for (int t = 0; t <= options.timesteps; t++) {
                for (int c = 0; c < interleaved_order.size(); c++) {
                    char interleved_char = interleaved_order[c];
                    for (int vt = 0; vt < char_tag_map[current_char].size(); vt++) {
                        variable_tag interleved_tag = char_tag_map[interleved_char][vt];
                        for (int j = 0; j < tagged_variables[interleved_tag][t].size(); j++) {
                            result.push_back(
                                std::make_tuple(tagged_variables[interleved_tag][t][j], custom_order_counter));
                        }
                    }
                }
                custom_order_counter++;  // increase counter after every timestep
            }
        } else {
            // add all timesteps for the corresponding var tag
            for (int vt = 0; vt < char_tag_map[current_char].size(); vt++) {
                variable_tag order_tag = char_tag_map[current_char][vt];
                for (int t = 0; t <= options.timesteps; t++) {
                    for (int j = 0; j < tagged_variables[order_tag][t].size(); j++) {
                        result.push_back(std::make_tuple(tagged_variables[order_tag][t][j], custom_order_counter));
                    }
                }
            }
            custom_order_counter++;  // increase counter for every group
        }
    }

    return result;
}

std::vector<int> order_variables_custom_force(encoder::encoder_abstract &encoder, option_values &options) {

    std::vector<std::tuple<int, int>> custom_order = create_custom_var_order_mapping(encoder, options);
    std::vector<std::tuple<int, int>> force_order = create_force_var_order_mapping(encoder, options);

    // combine tuples to triples
    std::vector<std::tuple<int, int, int>> combined_order;
    std::map<int, int> var_order_map; // helper map, maps var index to custom order
    for(int i = 0; i < custom_order.size(); i++){
        var_order_map[std::get<0>(custom_order[i])] = std::get<1>(custom_order[i]);
    }
    for(int i = 0; i < force_order.size(); i++){
        int var_idx = std::get<0>(force_order[i]);
        int force_idx = std::get<1>(force_order[i]);
        combined_order.push_back(std::make_tuple(var_idx, var_order_map[var_idx], force_idx)));
    }

    // sort by custom order and use force order as tiebreaker
    sort( combined_order.begin( ), combined_order.end( ), [ ]( const std::tuple<int, int, int>& lhs, const std::tuple<int, int, int>& rhs )
    {
        if(std::get<1>(lhs) == std::get<1>(rhs)){
            return std::get<2>(lhs) < std::get<2>(rhs);
        } else {
            return std::get<1>(lhs) < rstd::get<1>(rhs);
        }
    });
}

};  // namespace variable_order