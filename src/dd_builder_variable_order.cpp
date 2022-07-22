#include "dd_builder_variable_order.h"

#include "logging.h"
#include <iostream>
#include <set>

using namespace planning_logic;

namespace variable_order {

// used to interpret the order of clauses from the command line options
std::map<char, variable_tag> char_tag_map = {
    {'v', variable_plan_var},         {'o', variable_plan_op},          {'b', variable_plan_binary_op},
    {'h', variable_h_amost_variable}, {'j', variable_h_amost_operator}, {'k', variable_h_amost_mutex},
};

bool is_valid_variable_order_string(std::string build_order) {
    // check if string is permutation
    std::string standart_permutation = "vobx:hjk";
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
std::vector<int> put_variables_of_tag_first(cnf_encoder &encoder, std::vector<int> &current_order,
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

std::vector<int> order_variables(cnf_encoder &encoder, plan_to_cnf_map &symbol_map, option_values &options) {
    std::string build_order = options.variable_order;

    if (!is_valid_variable_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following variable order " << build_order;
        return std::vector<int>();
    }

    // categorize the variables
    categorized_variables tagged_variables = categorize_variables(symbol_map, options.timesteps);

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
            variable_tag order_tag = char_tag_map[current_char];

            // add all the variables for timestep t and tag order_tag
            interleved_variables.insert(interleved_variables.end(), tagged_variables[order_tag][t].begin(),
                                        tagged_variables[order_tag][t].end());
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
        variable_tag order_tag = char_tag_map[current_char];
        for (int t = 0; t <= options.timesteps; t++) {
            total_variables.insert(total_variables.end(), tagged_variables[order_tag][t].begin(),
                                   tagged_variables[order_tag][t].end());
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

};  // namespace variable_order