#include "dd_builder_conjoin_order.h"

#include "logging.h"

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

bool is_valid_conjoin_order_string(std::string build_order) {
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

std::vector<tagged_logic_primitiv> order_clauses(formula &cnf, option_values &options) {
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

            clause_tag order_tag = char_clause_tag_map[current_char];
            eo_constraint_tag constraint_order_tag = char_constraint_tag_map[current_char];
            tagged_clause curr_clause_category = std::make_tuple(order_tag, t);
            tagged_constraint curr_constraint_category = std::make_tuple(constraint_order_tag, t);

            // add all the clauses for timestep t and tag order_tag
            for (clause c : cnf.m_clause_map[curr_clause_category]) {
                interleved_clauses.push_back(std::make_pair(c, logic_clause));
            }
            // interleved_clauses.insert(interleved_clauses.end(), tagged_clauses[order_tag][t].begin(),
            //                           tagged_clauses[order_tag][t].end());
            //  add the exactly one constraints for the at_least_var or at_leat_op
            if (constraint_order_tag != eo_none) {
                for (eo_constraint e : cnf.m_eo_constraint_map[curr_constraint_category]) {
                    interleved_clauses.push_back(std::make_pair(e, logic_eo));
                }
            }
        }
    }

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];

        // add the interleved part
        if (current_char == 'x') {
            total_clauses.insert(total_clauses.end(), interleved_clauses.begin(), interleved_clauses.end());
            continue;
        }

        clause_tag order_tag = char_clause_tag_map[current_char];
        eo_constraint_tag constraint_order_tag = char_constraint_tag_map[current_char];

        // add all timesteps
        for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
            tagged_clause curr_clause_category = std::make_tuple(order_tag, t);
            for (clause c : cnf.m_clause_map[curr_clause_category]) {
                total_clauses.push_back(std::make_pair(c, logic_clause));
            }
        }

        // if exact one constraint
        if (constraint_order_tag != eo_none) {
            for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
                tagged_constraint curr_constraint_category = std::make_tuple(constraint_order_tag, t);
                for (eo_constraint e : cnf.m_eo_constraint_map[curr_constraint_category]) {
                    total_clauses.push_back(std::make_pair(e, logic_eo));
                }
            }
        }
    }

    // this reverses the order of the clauses. It allows the variables with the highes timesteps to be conjoined first
    if (options.reverse_order) {
        LOG_MESSAGE(log_level::info) << "Reversing order of the logic primitives";
        std::reverse(total_clauses.begin(), total_clauses.end());
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << total_clauses.size() << " clauses";
    return total_clauses;
}

std::vector<tagged_logic_primitiv> order_clauses_for_single_timestep(planning_logic::formula &cnf,
                                                                     option_values &options) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for single step bdd";
    std::string build_order = options.build_order;

    // TODO maybe do another check? only precon, effect and frame matters here
    // TODO maybe do an own oder string for layer by layer building
    if (!is_valid_conjoin_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following conjoin order " << build_order;
        return std::vector<tagged_logic_primitiv>();
    }

    // contains the result at the end
    std::vector<tagged_logic_primitiv> result_clauses;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    for (int i = 0; i < interleaved_order.size(); i++) {
        char current_char = interleaved_order[i];
        clause_tag order_tag = char_clause_tag_map[current_char];
        eo_constraint_tag constraint_order_tag = char_constraint_tag_map[current_char];
        if (order_tag != clause_precon && order_tag != clause_effect && order_tag != clause_frame) {
            continue;
        }
        LOG_MESSAGE(log_level::info) << "Ordering clauses of char " << current_char << " for single step bdd";
        tagged_clause curr_clause_category = std::make_tuple(order_tag, 0);

        // add all the clauses for timestep 0 and tag order_tag
        for (clause c : cnf.m_clause_map[curr_clause_category]) {
            result_clauses.push_back(std::make_pair(c, logic_clause));
        }
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << result_clauses.size() << " clauses for single step bdd";

    return result_clauses;
}

std::vector<tagged_logic_primitiv> order_clauses_for_no_timestep(planning_logic::formula &cnf, option_values &options) {
    LOG_MESSAGE(log_level::info) << "Ordering clauses for no step bdd";
    std::string build_order = options.build_order;

    // TODO maybe do another check? only precon, effect and frame matters here
    // TODO maybe do an own oder string for layer by layer building
    if (!is_valid_conjoin_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following conjoin order " << build_order;
        return std::vector<tagged_logic_primitiv>();
    }

    // contains the result at the end
    std::vector<tagged_logic_primitiv> result_clauses;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];
        if (current_char == 'x') {
            continue;
        }

        clause_tag order_tag = char_clause_tag_map[current_char];
        eo_constraint_tag constraint_order_tag = char_constraint_tag_map[current_char];

        // add all timesteps if it is no preco, eff, frame clause
        if (order_tag != clause_precon && order_tag != clause_effect && order_tag != clause_frame) {
            LOG_MESSAGE(log_level::info) << "Ordering clause of char " << current_char << " for no step bdd";
            for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
                tagged_clause curr_clause_category = std::make_tuple(order_tag, t);
                for (clause c : cnf.m_clause_map[curr_clause_category]) {
                    result_clauses.push_back(std::make_pair(c, logic_clause));
                }
            }
        }
        // if exact one constraint
        if (constraint_order_tag != eo_none) {
            LOG_MESSAGE(log_level::info) << "Ordering constraint of char " << current_char << " for no step bdd";
            for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
                tagged_constraint curr_constraint_category = std::make_tuple(constraint_order_tag, t);
                for (eo_constraint e : cnf.m_eo_constraint_map[curr_constraint_category]) {
                    result_clauses.push_back(std::make_pair(e, logic_eo));
                }
            }
        }
    }

    LOG_MESSAGE(log_level::info) << "Ordered a total of " << result_clauses.size()
                                 << " logic primitives for no step bdd";

    return result_clauses;
}

};  // namespace conjoin_order