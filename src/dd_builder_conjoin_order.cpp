#include "dd_builder_conjoin_order.h"

#include "logging.h"

using namespace planning_cnf;

namespace conjoin_order{

// used to interpret the order of clauses from the command line options
std::map<char, clause_tag> char_tag_map = {
    {'i', initial_state}, {'g', goal},  {'r', at_least_var}, {'t', at_most_var}, {'y', at_least_op},
    {'u', at_most_op},    {'m', mutex}, {'p', precondition}, {'e', effect},      {'c', changing_atoms},
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

categorized_clauses categorize_clauses(cnf &cnf) {
    LOG_MESSAGE(log_level::info) << "Starting to categorize clauses";

    // initilize the timestep buckets for the map
    categorized_clauses tagged_clauses;
    for (int tag_int = initial_state; tag_int <= none_clause; tag_int++) {
        clause_tag t = static_cast<clause_tag>(tag_int);

        // we only need one timestep
        if (t == initial_state || t == goal || t == none_clause) {
            tagged_clauses[t] = std::vector<std::vector<clause>>(1);
        }
        // we need one bucket for each timestep
        else {
            tagged_clauses[t] = std::vector<std::vector<clause>>(cnf.get_num_timesteps() + 1);
        }
    }

    // categorize clauses by tag and timestep
    for (int i = 0; i < cnf.get_num_clauses(); i++) {
        clause_tag t = cnf.get_tag(i);
        if (t == initial_state || t == goal) {
            tagged_clauses[t][0].push_back(cnf.get_clause(i));
        } else if (t == none_clause) {
            LOG_MESSAGE(log_level::error) << "Unknown tag during dd building: " << t;
            return std::map<clause_tag, std::vector<std::vector<clause>>>();
        } else {
            tagged_clauses[t][cnf.get_timestep(i)].push_back(cnf.get_clause(i));
        }
    }

    // print info about how many clauses each tag has
    for (int tag_int = initial_state; tag_int <= none_clause; tag_int++) {
        clause_tag t = static_cast<clause_tag>(tag_int);

        // we only need one timestep
        if (t == initial_state || t == goal || t == none_clause) {
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

std::vector<clause> order_clauses(cnf &cnf, std::string build_order) {

    if (!is_valid_conjoin_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following conjoin order " << build_order;
        return std::vector<clause>();
    }

    // categorize the clauses
    categorized_clauses tagged_clauses = categorize_clauses(cnf);

    // contains the result at the end
    std::vector<clause> interleved_clauses;
    std::vector<clause> total_clauses;

    // split the order into first and second part (in a really complicated manner)
    std::stringstream ss(build_order);
    std::string disjoin_order, interleaved_order;
    std::getline(ss, disjoin_order, ':');
    std::getline(ss, interleaved_order, ':');

    // sort the interleaved part
    for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
        for (int i = 0; i < interleaved_order.size(); i++) {
            char current_char = interleaved_order[i];
            clause_tag order_tag = char_tag_map[current_char];

            // add all the clauses for timestep t and tag order_tag
            interleved_clauses.insert(interleved_clauses.end(), tagged_clauses[order_tag][t].begin(),
                                      tagged_clauses[order_tag][t].end());
        }
    }

    for (int i = 0; i < disjoin_order.size(); i++) {
        char current_char = disjoin_order[i];

        // add the interleved part
        if (current_char == 'x') {
            total_clauses.insert(total_clauses.end(), interleved_clauses.begin(), interleved_clauses.end());
            continue;
        }

        clause_tag order_tag = char_tag_map[current_char];
        // only add first timestep
        if (order_tag == initial_state || order_tag == goal) {
            total_clauses.insert(total_clauses.end(), tagged_clauses[order_tag][0].begin(),
                                 tagged_clauses[order_tag][0].end());
        }
        // add all timesteps
        else {
            for (int t = 0; t <= cnf.get_num_timesteps(); t++) {
                total_clauses.insert(total_clauses.end(), tagged_clauses[order_tag][t].begin(),
                                     tagged_clauses[order_tag][t].end());
            }
        }
    }
    
    LOG_MESSAGE(log_level::info) << "Sorted a total of " << total_clauses.size() << " clauses";
    return total_clauses;
}

};