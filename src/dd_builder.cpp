#include "dd_builder.h"

#include "logging.h"

using namespace planning_cnf;

namespace dd_builder {

// used to interpret the order of clauses from the command line options
std::map<char, tag> char_tag_map = {
    {'i', initial_state}, {'g', goal},  {'r', at_least_var}, {'t', at_most_var}, {'y', at_least_op},
    {'u', at_most_op},    {'m', mutex}, {'p', precondition}, {'e', effect},      {'c', changing_atoms},
};

bool sanitize_build_order_string(std::string build_order) {
    // check if string is permutation
    std::string standart_permutation = "ixg:rtyumpec";
    if (!std::is_permutation(build_order.begin(), build_order.end(), standart_permutation.begin(),
                             standart_permutation.end())) {
        LOG_MESSAGE(log_level::warning) << "Build order has to be a permutation of 'ixg:rtyumpec' but is "
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
        LOG_MESSAGE(log_level::warning) << "First part of order has to contain i, g and x but is " << disjoin_order;
        return false;
    }

    return true;
}

std::map<tag, std::vector<std::vector<clause>>> categorize_clauses(cnf &cnf) {
    LOG_MESSAGE(log_level::info) << "Starting to categorize clauses";

    // initilize the timestep buckets for the map
    std::map<tag, std::vector<std::vector<clause>>> tagged_clauses;
    for (int tag_int = initial_state; tag_int <= none; tag_int++) {
        tag t = static_cast<tag>(tag_int);

        // we only need one timestep
        if (t == initial_state || t == goal || t == none) {
            tagged_clauses[t] = std::vector<std::vector<clause>>(1);
        }
        // we need one bucket for each timestep
        else {
            tagged_clauses[t] = std::vector<std::vector<clause>>(cnf.get_num_timesteps() + 1);
        }
    }

    // categorize clauses by tag and timestep
    for (int i = 0; i < cnf.get_num_clauses(); i++) {
        tag t = cnf.get_tag(i);
        if (t == initial_state || t == goal) {
            tagged_clauses[t][0].push_back(cnf.get_clause(i));
        } else if (t == none) {
            LOG_MESSAGE(log_level::error) << "Unknown tag during dd building: " << t;
            return std::map<tag, std::vector<std::vector<clause>>>();
        } else {
            tagged_clauses[t][cnf.get_timestep(i)].push_back(cnf.get_clause(i));
        }
    }

    // print info about how many clauses each tag has
    for (int tag_int = initial_state; tag_int <= none; tag_int++) {
        tag t = static_cast<tag>(tag_int);

        // we only need one timestep
        if (t == initial_state || t == goal || t == none) {
            LOG_MESSAGE(log_level::error) << "Categorized " << tagged_clauses[t][0].size() << " clauses of tag " << t;
        }
        // we need one bucket for each timestep
        else {
            int total_clauses = 0;
            for (int k = 0; k <= cnf.get_num_timesteps(); k++) {
                total_clauses += tagged_clauses[t][k].size();
            }
            LOG_MESSAGE(log_level::error) << "Categorized " << total_clauses << " clauses of tag " << t;
        }
    }

    return tagged_clauses;
}

std::vector<clause> sort_clauses(cnf &cnf, std::string build_order,
                                 std::map<tag, std::vector<std::vector<clause>>> &tagged_clauses) {
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
            tag order_tag = char_tag_map[current_char];

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

        tag order_tag = char_tag_map[current_char];
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

void construct_dd_linear_disjoint(dd_buildable &dd, cnf &cnf, std::string build_order, bool reversed) {
    LOG_MESSAGE(log_level::info) << "Construction DD with the following disjoint_order: " << build_order;

    if (!sanitize_build_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following order " << build_order;
        return;
    }

    // categorize clauses
    std::map<tag, std::vector<std::vector<clause>>> tagged_clauses = categorize_clauses(cnf);

    // sort the clauses (disjoint and interleaved)
    std::vector<clause> sorted_clauses = sort_clauses(cnf, build_order, tagged_clauses);

    // this reverses the order of the clauses. It allows the variables with the highes timesteps to be conjoined first
    if (reversed) {
        LOG_MESSAGE(log_level::info) << "Reversing order of the clauses";
        std::reverse(sorted_clauses.begin(), sorted_clauses.end());
    }

    // for every timestep (goal and inital only have one)
    // conjoin the clauses in the correct order
    for (int i = 0; i < sorted_clauses.size(); i++) {
        // for every clause in that timestep
        dd.conjoin_clause(sorted_clauses[i]);
        LOG_MESSAGE(log_level::info) << "Conjoined " << i + 1 << " clauses";
    }

    LOG_MESSAGE(log_level::info) << "Finished constructing DD";
}
}  // namespace dd_builder