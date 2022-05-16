#include "dd_builder_variable_order.h"

#include "logging.h"

using namespace planning_cnf;

namespace variable_order{

// used to interpret the order of clauses from the command line options
std::map<char, variable_tag> char_tag_map = {
    {'v', plan_variable}, {'o', plan_action},  {'h', h_amost_variable}, {'j', h_amost_operator}, {'k', h_amost_mutex},
};

bool is_valid_variable_order_string(std::string build_order) {
    // check if string is permutation
    std::string standart_permutation = "vox:hjk";
    if (!std::is_permutation(build_order.begin(), build_order.end(), standart_permutation.begin(),
                             standart_permutation.end())) {
        LOG_MESSAGE(log_level::error) << "Variable order has to be a permutation of " + standart_permutation + " but is "
                                        << build_order;
        return false;
    }
    return true;
}

categorized_variables categorize_variables(planning_cnf::cnf &cnf){
    LOG_MESSAGE(log_level::info) << "Starting to categorize variables";

    categorized_variables tagged_variables;

    // give every vector #timestpes+1 buckets
    for (int tag_int = initial_state; tag_int <= none_variable; tag_int++) {
        variable_tag t = static_cast<variable_tag>(tag_int);
        tagged_variables[t] = std::vector<std::vector<int>>(cnf.get_num_timesteps()+1);
    }

    // iterate over every variable of the cnf problem
    for (std::map<tagged_variable, int>::iterator iter = cnf.m_variable_map.begin(); iter != cnf.m_variable_map.end(); iter++) {
        
        // get information about the variable
        tagged_variable tag_var = iter->first;
        int cnf_index = iter->second;
        variable_tag tag = std::get<1>(tag_var);
        int index = std::get<0>(tag_var);
        int timestep = std::get<2>(tag_var);
        int value = std::get<3>(tag_var);
        LOG_MESSAGE(log_level::trace) << "Handeling variable idx:" << index << " tag:" << tag << " t:" << timestep << " val:" << value;

        tagged_variables[tag][timestep].push_back(cnf_index);
    }

    // print info about how many variables each tag has
    for (int tag_int = plan_variable; tag_int <= none_variable; tag_int++) {
        variable_tag t = static_cast<variable_tag>(tag_int);

        int total_variables = 0;
        for (int k = 0; k <= cnf.get_num_timesteps(); k++) {
            total_variables += tagged_variables[t][k].size();
        }
        LOG_MESSAGE(log_level::info) << "Categorized " << total_variables << " variables of tag " << t;
    }
    return tagged_variables;
}

std::vector<int> order_variables(planning_cnf::cnf &cnf, std::string build_order){
    // categorize the variables
    categorized_variables tagged_variables = categorize_variables(cnf);

    // contains the result at the end
    std::vector<int> interleved_variables;
    std::vector<int> total_variables;
    total_variables.push_back(0); // the 0th variable does not exist

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // sort the interleaved part
    for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
        for (int i = 0; i < interleaved_order.size(); i++) {
            char current_char = interleaved_order[i];
            variable_tag order_tag = char_tag_map[current_char];

            // add all the variables for timestep t and tag order_tag
            interleved_variables.insert(interleved_variables.end(), tagged_variables[order_tag][t].begin(),
                                      tagged_variables[order_tag][t].end());
        }
    }

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];

        // add the interleved part
        if (current_char == 'x') {
            total_variables.insert(total_variables.end(), interleved_variables.begin(), interleved_variables.end());
            continue;
        }
        // add all timesteps for the corresponding var tag
        variable_tag order_tag = char_tag_map[current_char];
        for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
            total_variables.insert(total_variables.end(), tagged_variables[order_tag][t].begin(),
                                 tagged_variables[order_tag][t].end());
        }
    }

    // total_clauses[i]: which cnf variable is at layer i
    // invert the map:
    std::vector<int> inverted_vector(total_variables.size());
    for(int i = 0; i < inverted_vector.size(); i++){
        inverted_vector[total_variables[i]] = i;
    }
    
    LOG_MESSAGE(log_level::info) << "Sorted a total of " << inverted_vector.size()-1 << " variables";
    return inverted_vector;
}

};