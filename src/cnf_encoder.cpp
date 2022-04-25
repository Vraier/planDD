#include "cnf_encoder.h"

#include <fstream>
#include <iostream>

#include "logging.h"

using namespace planning_cnf;

cnf cnf_encoder::encode_cnf(int timesteps) {
    LOG_MESSAGE(log_level::info) << "Start encoding SAS problem into CNF problem";

    generate_index_mapping(timesteps);

    m_cnf = cnf(m_symbol_map.size(), timesteps);

    LOG_MESSAGE(log_level::info) << "Starting to generate all clauses for the CNF problem";

    construct_initial_state_clauses();

    construct_goal_holds_clauses(timesteps);

    construct_at_least_on_value_clause(timesteps);

    construct_at_most_on_value_clause(timesteps);

    construct_at_least_one_action_clauses(timesteps);

    construct_at_most_one_action_clauses(timesteps);

    if(m_options.include_mutex){
        construct_mutex_clauses(timesteps);
    }

    construct_precondition_clauses(timesteps);

    construct_effect_clauses(timesteps);

    construct_changing_atom_implies_action_clauses(timesteps);

    LOG_MESSAGE(log_level::info) << "Constructed a total of " << m_cnf.get_num_clauses() << " clauses";

    return m_cnf;
}

int cnf_encoder::get_index(std::tuple<int, int, int> key) {
    if (m_symbol_map.find(key) == m_symbol_map.end()) {
        int size = m_symbol_map.size();
        m_symbol_map[key] = size + 1;
        return m_symbol_map[key];
    } else {
        return m_symbol_map[key];
    }
}

// Generates the mapping of symbol name to index
// generates timesetp + 1 variables for each symbol
// The initial state holds at timestep 0
void cnf_encoder::generate_index_mapping(int timesteps) {
    LOG_MESSAGE(log_level::info) << "Starting to generate a mapping of SAS variables to CNF variabels";
    for (int t = 0; t <= timesteps; t++) {
        // add all variables
        for (int var = 0; var < m_sas_problem.m_variabels.size(); var++) {
            // add all different variable values
            for (int val = 0; val < m_sas_problem.m_variabels[var].m_range; val++) {
                get_index(std::make_tuple(var, val, t));
            }
        }
    }

    // the operator used at timestep t (one less needed than variables)
    for (int t = 0; t < timesteps; t++) {
        // add all operators
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            get_index(std::make_tuple(op, -1, t));
        }
    }

    LOG_MESSAGE(log_level::info) << "Constructed a total of " << m_symbol_map.size() << " CNF variables";
}

// The initial state must hold at t = 0.
void cnf_encoder::construct_initial_state_clauses() {
    for (int var = 0; var < m_sas_problem.m_variabels.size(); var++) {
        for (int val = 0; val < m_sas_problem.m_variabels[var].m_range; val++) {
            std::vector<int> new_clause;
            int sym_index = get_index(std::make_tuple(var, val, 0));
            if (m_sas_problem.m_initial_state[var] == val) {
                new_clause.push_back(sym_index);
            } else {
                new_clause.push_back(-sym_index);
            }

            m_cnf.add_clause(new_clause, initial_state, 0);
        }
    }
}

// At every timestep a sas variable has at least one value
void cnf_encoder::construct_at_least_on_value_clause(int timesteps) {
    for (int t = 0; t <= timesteps; t++) {
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
            std::vector<int> new_clause;
            for (int val = 0; val < m_sas_problem.m_variabels[v].m_range; val++) {
                int var_index = get_index(std::make_tuple(v, val, t));
                new_clause.push_back(var_index);
            }
            m_cnf.add_clause(new_clause, at_least_var, t);
        }
    }
}

// At every step, a sas varaibel has at most one value
void cnf_encoder::construct_at_most_on_value_clause(int timesteps) {
    for (int t = 0; t <= timesteps; t++) {
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
            for (int val1 = 0; val1 < m_sas_problem.m_variabels[v].m_range; val1++) {
                for (int val2 = val1 + 1; val2 < m_sas_problem.m_variabels[v].m_range; val2++) {
                    int index1, index2;
                    index1 = get_index(std::make_tuple(v, val1, t));
                    index2 = get_index(std::make_tuple(v, val2, t));

                    std::vector<int> new_clause;
                    new_clause.push_back(-index1);
                    new_clause.push_back(-index2);

                    m_cnf.add_clause(new_clause, at_most_var, t);
                }
            }
        }
    }
}

// At every step, at least one action is applied
void cnf_encoder::construct_at_least_one_action_clauses(int timesteps) {
    for (int t = 0; t < timesteps; t++) {
        std::vector<int> new_clause;
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int sym_index = get_index(std::make_tuple(op, -1, t));
            new_clause.push_back(sym_index);
        }

        m_cnf.add_clause(new_clause, at_least_op, t);
    }
}

// At every step, at most one action is applied
void cnf_encoder::construct_at_most_one_action_clauses(int timesteps) {
    for (int t = 0; t < timesteps; t++) {
        for (int op1 = 0; op1 < m_sas_problem.m_operators.size(); op1++) {
            for (int op2 = op1 + 1; op2 < m_sas_problem.m_operators.size(); op2++) {
                int index1, index2;
                index1 = get_index(std::make_tuple(op1, -1, t));
                index2 = get_index(std::make_tuple(op2, -1, t));

                std::vector<int> new_clause;
                new_clause.push_back(-index1);
                new_clause.push_back(-index2);

                m_cnf.add_clause(new_clause, at_most_op, t);
            }
        }
    }
}

// If action a is applied at step t, then pre(a) holds at step t.
// TODO: i am not sure if this is correct (i didnt use every information from
// the operators)
void cnf_encoder::construct_precondition_clauses(int timesteps) {
    for (int t = 0; t < timesteps; t++) {
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            for (int eff = 0; eff < m_sas_problem.m_operators[op].m_effects.size(); eff++) {
                int effected_var, effected_old_val;
                effected_var = std::get<0>(m_sas_problem.m_operators[op].m_effects[eff]);
                effected_old_val = std::get<1>(m_sas_problem.m_operators[op].m_effects[eff]);
                if (effected_old_val == -1) {
                    // a value of -1 the value of the variable is irrelevant,
                    // when determining if it is applicable
                    continue;
                }

                int index_op, index_precondition;
                index_op = get_index(std::make_tuple(op, -1, t));
                index_precondition = get_index(std::make_tuple(effected_var, effected_old_val, t));

                std::vector<int> new_clause;
                new_clause.push_back(-index_op);
                new_clause.push_back(index_precondition);

                m_cnf.add_clause(new_clause, precondition, t);
            }
        }
    }
}

// If action a is applied at step t, then eff(a) hold at step t + 1.
void cnf_encoder::construct_effect_clauses(int timesteps) {
    for (int t = 0; t < timesteps; t++) {
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            for (int eff = 0; eff < m_sas_problem.m_operators[op].m_effects.size(); eff++) {
                int effected_var, effected_new_val;
                effected_var = std::get<0>(m_sas_problem.m_operators[op].m_effects[eff]);
                effected_new_val = std::get<2>(m_sas_problem.m_operators[op].m_effects[eff]);

                int index_op, index_effect;
                index_op = get_index(std::make_tuple(op, -1, t));
                index_effect = get_index(std::make_tuple(effected_var, effected_new_val, t + 1));

                std::vector<int> new_clause;
                new_clause.push_back(-index_op);
                new_clause.push_back(index_effect);

                m_cnf.add_clause(new_clause, effect, t);
            }
        }
    }
}

// The goal g holds at step n
void cnf_encoder::construct_goal_holds_clauses(int timesteps) {
    for (int g = 0; g < m_sas_problem.m_goal.size(); g++) {
        std::vector<int> new_clause;
        std::pair<int, int> goal_value = m_sas_problem.m_goal[g];
        int index_var = get_index(std::make_tuple(goal_value.first, goal_value.second, timesteps));

        new_clause.push_back(index_var);
        m_cnf.add_clause(new_clause, goal, timesteps);
    }
}

// If atom p changes between steps t and t + 1, an action which supports this
// change must be applied at t:
void cnf_encoder::construct_changing_atom_implies_action_clauses(int timesteps) {
    for (int t = 0; t < timesteps; t++) {                             // for every timestep
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {  // for every variable

            // find all possible value changes of a variable
            int domain_size = m_sas_problem.m_variabels[v].m_range;
            for (int val1 = 0; val1 < domain_size; val1++) {
                for (int val2 = 0; val2 < domain_size; val2++) {
                    if (val1 == val2) continue;  // skip if no value has changes

                    std::vector<int> new_clause;
                    int index_val1, index_val2;
                    index_val1 = get_index(std::make_tuple(v, val1, t));
                    index_val2 = get_index(std::make_tuple(v, val2, t + 1));
                    new_clause.push_back(-index_val1);
                    new_clause.push_back(-index_val2);

                    // find the actions that support this transition.
                    for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
                        for (int i = 0; i < m_sas_problem.m_operators[op].m_effects.size(); i++) {
                            std::tuple<int, int, int> op_eff = m_sas_problem.m_operators[op].m_effects[i];
                            int effected_var, val_pre, val_post;
                            effected_var = std::get<0>(op_eff);
                            val_pre = std::get<1>(op_eff);
                            val_post = std::get<2>(op_eff);

                            // check if corrected variable is affected, it
                            // changes to the right value, and it changes from
                            // the right value if everything is met, add it to
                            // the clause
                            if ((effected_var == v) && (val_post == val2) && ((val_pre == val1) || (val_pre == -1))) {
                                int index_possible_op = get_index(std::make_tuple(op, -1, t));
                                new_clause.push_back(index_possible_op);
                                // break; // this is a fishy break :D, therfore
                                // i comment it out. Be we dont have to check
                                // this operator anymore because it is already a
                                // candidate
                            }
                        }
                    }

                    m_cnf.add_clause(new_clause, changing_atoms, t);
                }
            }
        }
    }
}

// at every timestep it is not allowed for two values in a mutex to be true at the same time
void cnf_encoder::construct_mutex_clauses(int timesteps){
    for (int t = 0; t <= timesteps; t++) {
        for (int m = 0; m < m_sas_problem.m_mutex_groups.size(); m++) {
            for (int i = 0; i < m_sas_problem.m_mutex_groups[m].size(); i++) {
                for (int j = i + 1; j < m_sas_problem.m_mutex_groups[m].size(); j++) {
                    std::pair<int, int> pair1, pair2;
                    pair1 = m_sas_problem.m_mutex_groups[m][i];
                    pair2 = m_sas_problem.m_mutex_groups[m][j];

                    int index1, index2;
                    index1 = get_index(std::make_tuple(pair1.first, pair1.second, t));
                    index2 = get_index(std::make_tuple(pair2.first, pair2.second, t));

                    std::vector<int> new_clause;
                    new_clause.push_back(-index1);
                    new_clause.push_back(-index2);

                    m_cnf.add_clause(new_clause, mutex, t);
                }
            }
        }
    }
}

void cnf_encoder::write_cnf_to_file(std::string filepath, cnf &cnf) {
    std::fstream file_out;
    file_out.open(filepath, std::ios_base::out);

    file_out << "p cnf " << cnf.get_num_variables() << " " << cnf.get_num_clauses() << std::endl;
    for (int i = 0; i < cnf.get_num_clauses(); i++) {
        std::vector<int> clause = cnf.get_clause(i);
        for (int l : clause) {
            file_out << l << " ";
        }
        file_out << "0" << std::endl;
    }
}

// This call depends on the correct symbol map.
std::vector<bool> cnf_encoder::parse_cnf_solution(std::string filepath) {
    LOG_MESSAGE(log_level::info) << "Start parsing assignment file";

    std::ifstream infile(filepath);
    std::string line;
    std::istringstream iss;
    std::vector<bool> assignment(m_symbol_map.size(), false);
    assignment[0] = false;

    std::getline(infile, line);
    if (line != "SAT") {
        LOG_MESSAGE(log_level::info) << "Assignemnt file contains no solution" << std::endl;
        return std::vector<bool>();
    }
    std::getline(infile, line);
    iss = std::istringstream(line);

    int int_val;
    iss >> int_val;

    while (int_val != 0) {
        int index = std::abs(int_val);
        if (index > m_symbol_map.size()) {
            LOG_MESSAGE(log_level::error) << "The assignment values are too big. Symbol map has size "
                                          << m_symbol_map.size() << " the assignment has value" << int_val;
        }
        bool bool_val = (int_val > 0) ? true : false;
        assignment[index] = bool_val;

        iss >> int_val;
    }

    return assignment;
}

// interprets a solution from minisat. It translates the sat solution to a
// planning problem solution (with some addional debugg information)
// This call depends on the correct symbol map.
void cnf_encoder::decode_cnf_solution(std::vector<bool> &assignment, int timesteps) {
    if (assignment.size() == 0) {
        LOG_MESSAGE(log_level::warning) << "Trying to decode an assignment of size 0";
        return;
    }
    for (int t = 0; t <= timesteps; t++) {
        // print information about the state
        std::cout << "==========================================" << std::endl;
        std::cout << "Step " << t << " has the following state:" << std::endl;
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
            for (int val = 0; val < m_sas_problem.m_variabels[v].m_range; val++) {
                int idx = get_index(std::make_tuple(v, val, t));
                if (assignment[idx]) {
                    std::cout << m_sas_problem.m_variabels[v].m_name << ": "
                              << m_sas_problem.m_variabels[v].m_symbolic_names[val] << std::endl;
                }
            }
        }

        // print information about the operators used
        if (t == timesteps) continue;  // dont print an operator for the last state (there is none)
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int idx = get_index(std::make_tuple(op, -1, t));
            if (assignment[idx]) {
                std::cout << "operator: " << m_sas_problem.m_operators[op].m_name << std::endl;
            }
        }
    }

    std::cout << "The goal variables are the following:" << std::endl;
    for (int g = 0; g < m_sas_problem.m_goal.size(); g++) {
        int goal_var, goal_val, goal_idx;
        goal_var = m_sas_problem.m_goal[g].first;
        goal_val = m_sas_problem.m_goal[g].second;
        goal_idx = get_index(std::make_tuple(goal_var, goal_val, timesteps));
        std::cout << "Variable " << m_sas_problem.m_variabels[goal_var].m_name << " has value "
                  << m_sas_problem.m_variabels[goal_var].m_symbolic_names[goal_val];
        if (assignment[goal_idx]) {
            std::cout << " it holds" << std::endl;
        } else {
            std::cout << " it does not hold" << std::endl;
        }
    }
}

void cnf_encoder::compare_assignments(std::vector<bool> &assignment1, std::vector<bool> &assignment2) {
    LOG_MESSAGE(log_level::info) << "Comparing two assignments. Size of symbol_map and assignemtn one and two is: "
                                 << m_symbol_map.size() << " " << assignment1.size() << " " << assignment2.size();

    for (int i = 0; i < m_symbol_map.size(); i++) {
        // search for cnf values that changes between the assignments
        if (assignment1[i] != assignment2[i]) {
            for (auto &it : m_symbol_map) {
                if (it.second == i) {
                    std::tuple<int, int, int> changed_val = it.first;
                    std::string changed_name;
                    if (std::get<1>(changed_val) == -1) {
                        // represents operator
                        changed_name = m_sas_problem.m_operators[std::get<0>(changed_val)].m_name;
                    } else {
                        // represents variable
                        changed_name = m_sas_problem.m_variabels[std::get<0>(changed_val)]
                                           .m_symbolic_names[std::get<1>(changed_val)];
                    }

                    std::cout << "Changed the cnf variable " << i << " from "
                              << (assignment1[i] ? "true to false" : "false to true") << " it represents value "
                              << changed_name << " t=" << std::get<2>(changed_val) << std::endl;
                }
            }
        }
    }
}