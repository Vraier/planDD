#include "dd_builder_conjoin_order.h"

#include "logging.h"

using namespace planning_logic;

namespace conjoin_order {

// used to interpret the order of clauses from the command line options
std::map<char, clause_tag> char_clause_tag_map = {
    {'i', clause_ini_state}, {'g', clause_goal},  {'r', clause_al_var}, {'t', clause_am_var}, {'y', clause_al_op},
    {'u', clause_am_op},    {'m', clause_mutex}, {'p', clause_precon}, {'e', clause_effect},      {'c', clause_frame},
};
std::map<char, eo_constraint_tag> char_constraint_tag_map = {
    {'i', eo_none}, {'g', eo_none}, {'r', eo_var},   {'t', eo_none},
    {'y', eo_op},    {'u', eo_none}, {'m', eo_none}, {'p', eo_none},
    {'e', eo_none}, {'c', eo_none},
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

categorized_clauses categorize_clauses(formula &cnf) {
    LOG_MESSAGE(log_level::info) << "Starting to categorize clauses";

    // initilize the timestep buckets for the map
    categorized_clauses tagged_clauses;
    for (int tag_int = clause_ini_state; tag_int <= clause_none; tag_int++) {
        clause_tag t = static_cast<clause_tag>(tag_int);

        // we only need one timestep
        if (t == clause_ini_state || t == clause_goal || t == clause_none) {
            tagged_clauses[t] = std::vector<std::vector<clause>>(1);
        }
        // we need one bucket for each timestep
        else {
            tagged_clauses[t] = std::vector<std::vector<clause>>(cnf.get_num_timesteps() + 1);
        }
    }

    // categorize clauses by tag and timestep
    for (int i = 0; i < cnf.get_num_clauses(); i++) {
        clause_tag t = cnf.get_clause_tag(i);
        if (t == clause_ini_state || t == clause_goal) {
            tagged_clauses[t][0].push_back(cnf.get_clause(i));
        } else if (t == clause_none) {
            LOG_MESSAGE(log_level::error) << "Unknown tag during dd building: " << t;
            return std::map<clause_tag, std::vector<std::vector<clause>>>();
        } else {
            tagged_clauses[t][cnf.get_clause_timestep(i)].push_back(cnf.get_clause(i));
        }
    }

    // print info about how many clauses each tag has
    for (int tag_int = clause_ini_state; tag_int <= clause_none; tag_int++) {
        clause_tag t = static_cast<clause_tag>(tag_int);

        // we only need one timestep
        if (t == clause_ini_state || t == clause_goal || t == clause_none) {
            LOG_MESSAGE(log_level::info) << "Categorized " << tagged_clauses[t][0].size() << " clauses of tag " << t;
        }
        // we need one bucket for each timestep
        else {
            int total_clauses = 0;
            for (int k = 0; k <= cnf.get_num_timesteps(); k++) {
                total_clauses += tagged_clauses[t][k].size();
            }
            LOG_MESSAGE(log_level::info) << "Categorized " << total_clauses << " clauses of tag " << t;
        }
    }

    return tagged_clauses;
}

categorized_constraints categorize_constraints(formula &cnf) {
    LOG_MESSAGE(log_level::info) << "Starting to categorize clauses";

    // initilize the timestep buckets for the map
    categorized_constraints tagged_constraints;
    for (int tag_int = eo_var; tag_int <= eo_none; tag_int++) {
        eo_constraint_tag t = static_cast<eo_constraint_tag>(tag_int);

        tagged_constraints[t] = std::vector<std::vector<eo_constraint>>(cnf.get_num_timesteps() + 1);
    }

    // categorize clauses by tag and timestep
    for (int i = 0; i < cnf.get_num_constraints(); i++) {
        eo_constraint_tag t = cnf.get_constraint_tag(i);
        tagged_constraints[t][cnf.get_constraint_timestep(i)].push_back(cnf.get_constraint(i));
    }

    // print info about how many clauses each tag has
    for (int tag_int = eo_var; tag_int <= eo_none; tag_int++) {
        eo_constraint_tag t = static_cast<eo_constraint_tag>(tag_int);

        int total_constraints = 0;
        for (int k = 0; k <= cnf.get_num_timesteps(); k++) {
            total_constraints += tagged_constraints[t][k].size();
        }
        LOG_MESSAGE(log_level::info) << "Categorized " << total_constraints << " constraints of tag " << t;
    }
    return tagged_constraints;
}

std::vector<tagged_logic_primitiv> order_clauses(formula &cnf, option_values &options) {
    std::string build_order = options.build_order;

    if (!is_valid_conjoin_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following conjoin order " << build_order;
        return std::vector<tagged_logic_primitiv>();
    }

    // categorize the clauses and constraints
    categorized_clauses tagged_clauses = categorize_clauses(cnf);
    categorized_constraints tagged_constraints = categorize_constraints(cnf);

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

            // add all the clauses for timestep t and tag order_tag
            for (clause c : tagged_clauses[order_tag][t]) {
                interleved_clauses.push_back(std::make_pair(c, logic_clause));
            }
            // interleved_clauses.insert(interleved_clauses.end(), tagged_clauses[order_tag][t].begin(),
            //                           tagged_clauses[order_tag][t].end());
            //  add the exactly one constraints for the at_least_var or at_leat_op
            if (constraint_order_tag != eo_none) {
                for (eo_constraint e : tagged_constraints[constraint_order_tag][t]) {
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
        // only add first timestep
        if (order_tag == clause_ini_state || order_tag == clause_goal) {
            for (clause c : tagged_clauses[order_tag][0]) {
                total_clauses.push_back(std::make_pair(c, logic_clause));
            }
        }
        // add all timesteps
        else {
            for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
                for (clause c : tagged_clauses[order_tag][t]) {
                    total_clauses.push_back(std::make_pair(c, logic_clause));
                }
            }
        }
        // if exact one constraint
        if (constraint_order_tag != eo_none) {
            for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
                for (eo_constraint e : tagged_constraints[constraint_order_tag][t]) {
                    total_clauses.push_back(std::make_pair(e, logic_eo));
                }
            }
        }
    }

    LOG_MESSAGE(log_level::info) << "Sorted a total of " << total_clauses.size() << " clauses";
    return total_clauses;
}

};  // namespace conjoin_order