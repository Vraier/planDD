#include "dd_builder_conjoin_order.h"

#include "logging.h"

#include <algorithm>

using namespace planning_logic;

namespace conjoin_order {

// used to interpret the order of clauses from the command line options
std::map<char, clause_tag> char_clause_tag_map = {
    {'i', clause_ini_state}, {'g', clause_goal},  {'r', clause_al_var}, {'t', clause_am_var}, {'y', clause_al_op},
    {'u', clause_am_op},     {'m', clause_mutex}, {'p', clause_precon}, {'e', clause_effect}, {'c', clause_frame},
};
std::map<char, eo_constraint_tag> char_constraint_tag_map = {
    {'i', eo_none}, {'g', eo_none}, {'r', eo_var},  {'t', eo_none}, {'y', eo_op},
    {'u', eo_none}, {'m', eo_none}, {'p', eo_none}, {'e', eo_none}, {'c', eo_none},
};

bool is_valid_conjoin_order_string(std::string &build_order) {
    // check if string is permutation
    std::string standart_permutation = "grtyumix:pec";
    if (!std::is_permutation(build_order.begin(), build_order.end(), standart_permutation.begin(),
                             standart_permutation.end())) {
        LOG_MESSAGE(log_level::error) << "Build order has to be a permutation of " + standart_permutation + " but is "
                                      << build_order;
        return false;
    }

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // check if first part contains x, i and g
    if (disjoin_order.find("x") == std::string::npos || disjoin_order.find("i") == std::string::npos ||
        disjoin_order.find("g") == std::string::npos) {
        LOG_MESSAGE(log_level::error) << "First part of order has to contain i, g and x but is " << disjoin_order;
        return false;
    }

    return true;
}

int count_char(std::string str, char c){
    int count = 0;
    for(char a: str){
        if (a == c) {
            count++;
        }
    }
    return count;
}

bool is_valid_layer_order_string(std::string &build_order) {
    // check if string is permutation
    int num_colons = count_char(build_order, ':');
    if(num_colons != 2){
        LOG_MESSAGE(log_level::error) << "Build contains wrong number of colons (must be 2)";
        return false;
    }
    return true;
}

void print_info_about_number_of_logic_primitives(formula &cnf) {
    // print info about how many clauses each tag has
    for (int tag_int = clause_ini_state; tag_int <= clause_none; tag_int++) {
        clause_tag tag = static_cast<clause_tag>(tag_int);

        int total_clauses = 0;
        for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
            total_clauses += cnf.m_clause_map[std::make_tuple(tag, t)].size();
        }
        LOG_MESSAGE(log_level::info) << "Categorized " << total_clauses << " clauses of tag " << tag;
    }

    // print info about how many constraints each tag has
    for (int tag_int = eo_var; tag_int <= eo_none; tag_int++) {
        eo_constraint_tag tag = static_cast<eo_constraint_tag>(tag_int);

        int total_constraints = 0;
        for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
            total_constraints += cnf.m_eo_constraint_map[std::make_tuple(tag, t)].size();
        }
        LOG_MESSAGE(log_level::info) << "Categorized " << total_constraints << " constraints of tag " << tag;
    }
}

std::vector<tagged_logic_primitiv> order_all_clauses(formula &cnf, option_values &options) {
    std::string build_order = options.build_order;

    if (!is_valid_conjoin_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following conjoin order " << build_order;
        return std::vector<tagged_logic_primitiv>();
    }

    print_info_about_number_of_logic_primitives(cnf);

    // contains the result at the end
    std::vector<tagged_logic_primitiv> interleved_clauses;
    std::vector<tagged_logic_primitiv> total_clauses;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // sort the interleaved part
    for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
        for (int i = 0; i < interleaved_order.size(); i++) {
            char current_char = interleaved_order[i];

            std::vector<tagged_logic_primitiv> temp_clauses =
                collect_primitives_for_single_timestep(cnf, current_char, t);
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

        std::vector<tagged_logic_primitiv> temp_clauses = collect_primitives_for_all_timesteps(cnf, current_char);
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

std::vector<tagged_logic_primitiv> order_clauses_for_layer(planning_logic::formula &cnf, int layer, std::string &order_string) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for single layer " << layer;

    // contains the result at the end
    std::vector<tagged_logic_primitiv> result_clauses;
    std::vector<tagged_logic_primitiv> temp_clauses;

    for (char c : order_string) {
        temp_clauses = collect_primitives_for_single_timestep(cnf, c, layer);
        // if we add at least or at most one var clauses, also do it for the second timestep
        if (c == 'r' || c == 't' || c == 'y' || c == 'u'|| c == 'm') {
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

std::vector<tagged_logic_primitiv> order_clauses_for_foundation(planning_logic::formula &cnf, std::string &order_string) {
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

std::vector<tagged_logic_primitiv> collect_primitives_for_all_timesteps(formula &cnf, char primitive_type) {
    std::vector<tagged_logic_primitiv> result_primitives;
    clause_tag clause_order_tag = char_clause_tag_map[primitive_type];
    eo_constraint_tag constraint_order_tag = char_constraint_tag_map[primitive_type];

    for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
        tagged_clause curr_clause_category = std::make_tuple(clause_order_tag, t);
        tagged_constraint curr_constraint_category = std::make_tuple(constraint_order_tag, t);

        // add all the clauses for timestep t and tag order_tag
        for (clause c : cnf.m_clause_map[curr_clause_category]) {
            result_primitives.push_back(std::make_pair(c, logic_clause));
        }
        //  add the exactly one constraints for the at_least_var or at_leat_op
        for (eo_constraint c : cnf.m_eo_constraint_map[curr_constraint_category]) {
            result_primitives.push_back(std::make_pair(c, logic_eo));
        }
    }

    return result_primitives;
}

std::vector<tagged_logic_primitiv> collect_primitives_for_single_timestep(formula &cnf, char primitive_type,
                                                                          int timestep) {
    std::vector<tagged_logic_primitiv> result_primitives;
    clause_tag clause_order_tag = char_clause_tag_map[primitive_type];
    eo_constraint_tag constraint_order_tag = char_constraint_tag_map[primitive_type];

    tagged_clause curr_clause_category = std::make_tuple(clause_order_tag, timestep);
    tagged_constraint curr_constraint_category = std::make_tuple(constraint_order_tag, timestep);

    // add all the clauses for timestep t and tag order_tag
    for (clause c : cnf.m_clause_map[curr_clause_category]) {
        result_primitives.push_back(std::make_pair(c, logic_clause));
    }
    //  add the exactly one constraints for the at_least_var or at_leat_op
    for (eo_constraint c : cnf.m_eo_constraint_map[curr_constraint_category]) {
        result_primitives.push_back(std::make_pair(c, logic_eo));
    }

    return result_primitives;
}

};  // namespace conjoin_order