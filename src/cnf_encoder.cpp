#include "cnf_encoder.h"

#include <fstream>
#include <iostream>

#include "logging.h"

using namespace planning_cnf;


cnf cnf_encoder::encode_cnf(int timesteps) {
    LOG_MESSAGE(log_level::info) << "Start encoding SAS problem into CNF problem";
    LOG_MESSAGE(log_level::info) << "Starting to generate all clauses for the CNF problem";

    m_cnf = cnf(timesteps);

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
    LOG_MESSAGE(log_level::info) << "Constructed a total of " << m_cnf.get_num_variables() << " variables (with helper)";

    return m_cnf;
}

// TODO check if 6 is correct magic number
std::vector<std::vector<int>> cnf_encoder::generate_at_most_one_constraint(std::vector<int> &variables, variable_tag constraint_type, int timestep){

    if(!m_options.use_ladder_encoding) {
        return generate_at_most_one_constraint_pairwise(variables);
    }
    else if(m_options.use_ladder_encoding && variables.size() <=6 ) {
        return generate_at_most_one_constraint_pairwise(variables);
    }
    else {
        return generate_at_most_one_constraint_ladder(variables, constraint_type, timestep);
    }
}

std::vector<std::vector<int>> cnf_encoder::generate_at_most_one_constraint_ladder(std::vector<int> &variables, variable_tag constraint_type, int timestep){
    std::vector<std::vector<int>> all_new_clauses;

    for(int i = 0; i < variables.size(); i++){
        if(i != 0 && i != variables.size()-1) { // first and last helper variable dont need this clause
            std::vector<int> new_clause;
            new_clause.push_back(-m_cnf.get_variable_index(i-1, constraint_type, timestep)); // !si-1
            new_clause.push_back( m_cnf.get_variable_index(i,   constraint_type, timestep)); //si
            all_new_clauses.push_back(new_clause);
        }
        if (i != variables.size()-1) { // last variable does not need this implication
            std::vector<int> new_clause;
            new_clause.push_back(-variables[i]); // !Xi
            new_clause.push_back( m_cnf.get_variable_index(i, constraint_type, timestep)); // si
            all_new_clauses.push_back(new_clause);
        }
        if( i != 0) { // first vaiable does not need this implication
            std::vector<int> new_clause;
            new_clause.push_back(-variables[i]); // !Xi
            new_clause.push_back(-m_cnf.get_variable_index(i-1, constraint_type, timestep)); // !si-1
            all_new_clauses.push_back(new_clause);
        }
    }

    return all_new_clauses;
}

std::vector<std::vector<int>> cnf_encoder::generate_at_most_one_constraint_pairwise(std::vector<int> &variables){
    std::vector<std::vector<int>> all_new_clauses;
    for(int i = 0; i < variables.size(); i++){
        for(int j = i+1; j < variables.size(); j++) {
            std::vector<int> new_clause;
            new_clause.push_back(-variables[i]); // !Xi
            new_clause.push_back(-variables[j]); // !Xj
            all_new_clauses.push_back(new_clause);
        }
    }
    return all_new_clauses;
}

// The initial state must hold at t = 0.
void cnf_encoder::construct_initial_state_clauses() {
    for (int var = 0; var < m_sas_problem.m_variabels.size(); var++) {
        for (int val = 0; val < m_sas_problem.m_variabels[var].m_range; val++) {
            std::vector<int> new_clause;
            int sym_index = m_cnf.get_variable_index(var, plan_variable, 0, val);
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
                int var_index = m_cnf.get_variable_index(v, plan_variable, t, val);
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

            std::vector<int> at_most_one_should_be_true;
            for(int val = 0; val < m_sas_problem.m_variabels[v].m_range; val++){
                int index = m_cnf.get_variable_index(v, plan_variable, t, val);
                at_most_one_should_be_true.push_back(index);
            }

            std::vector<std::vector<int>> constrain_clauses = generate_at_most_one_constraint(at_most_one_should_be_true, h_amost_variable, t);
            for(std::vector<int> constraint: constrain_clauses) {
                m_cnf.add_clause(constraint, at_most_var, t);
            }
        }
    }
}

// At every step, at least one action is applied
void cnf_encoder::construct_at_least_one_action_clauses(int timesteps) {
    for (int t = 0; t < timesteps; t++) {
        std::vector<int> new_clause;
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int sym_index = m_cnf.get_variable_index(op, plan_action, t);
            new_clause.push_back(sym_index);
        }

        m_cnf.add_clause(new_clause, at_least_op, t);
    }
}

// At every step, at most one action is applied
void cnf_encoder::construct_at_most_one_action_clauses(int timesteps) {
    for (int t = 0; t < timesteps; t++) {

        std::vector<int> at_most_one_should_be_true;
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int index = m_cnf.get_variable_index(op, plan_action, t);
            at_most_one_should_be_true.push_back(index);
        }

        std::vector<std::vector<int>> constrain_clauses = generate_at_most_one_constraint(at_most_one_should_be_true, h_amost_operator, t);
        for(std::vector<int> constraint: constrain_clauses) {
            m_cnf.add_clause(constraint, at_most_op, t);
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
                index_op = m_cnf.get_variable_index(op, plan_action, t);
                index_precondition = m_cnf.get_variable_index(effected_var, plan_variable, t, effected_old_val);

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
                index_op = m_cnf.get_variable_index(op, plan_action, t);
                index_effect = m_cnf.get_variable_index(effected_var, plan_variable,  t + 1, effected_new_val);

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
        int index_var = m_cnf.get_variable_index(goal_value.first, plan_variable, timesteps, goal_value.second);

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
                    index_val1 = m_cnf.get_variable_index(v, plan_variable, t, val1);
                    index_val2 = m_cnf.get_variable_index(v, plan_variable, t + 1, val2);
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
                                int index_possible_op = m_cnf.get_variable_index(op, plan_action, t);
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


            std::vector<int> at_most_one_should_be_true;
            for (int i = 0; i < m_sas_problem.m_mutex_groups[m].size(); i++) {
                std::pair<int, int> var_val_pair = m_sas_problem.m_mutex_groups[m][i];
                int index = m_cnf.get_variable_index(var_val_pair.first, plan_variable, t, var_val_pair.second);
                at_most_one_should_be_true.push_back(index);
            }

            std::vector<std::vector<int>> constrain_clauses = generate_at_most_one_constraint(at_most_one_should_be_true, h_amost_mutex, t);
            for(std::vector<int> constraint: constrain_clauses) {
                m_cnf.add_clause(constraint, mutex, t);
            }
        }
    }
}

// This call depends on the correct symbol map.
std::vector<bool> cnf_encoder::parse_cnf_solution(std::string filepath) {
    LOG_MESSAGE(log_level::info) << "Start parsing assignment file";

    std::ifstream infile(filepath);
    std::string line;
    std::istringstream iss;
    std::vector<bool> assignment(m_cnf.get_num_variables(), false);
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
        if (index > m_cnf.get_num_variables()) {
            LOG_MESSAGE(log_level::error) << "The assignment values are too big. Symbol map has size "
                                          << m_cnf.get_num_variables() << " the assignment has value" << int_val;
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
void cnf_encoder::decode_cnf_solution(std::vector<bool> &assignment) {
    if (assignment.size() == 0) {
        LOG_MESSAGE(log_level::warning) << "Trying to decode an assignment of size 0";
        return;
    }
    for (int t = 0; t <= m_cnf.get_num_timesteps(); t++) {
        // print information about the state
        std::cout << "==========================================" << std::endl;
        std::cout << "Step " << t << " has the following state:" << std::endl;
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
            for (int val = 0; val < m_sas_problem.m_variabels[v].m_range; val++) {
                int idx = m_cnf.get_variable_index_without_adding(v, plan_variable, t, val);
                if (assignment[idx]) {
                    std::cout << m_sas_problem.m_variabels[v].m_name << ": "
                              << m_sas_problem.m_variabels[v].m_symbolic_names[val] << std::endl;
                }
            }
        }

        // print information about the operators used
        if (t == m_cnf.get_num_timesteps()) continue;  // dont print an operator for the last state (there is none)
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int idx = m_cnf.get_variable_index_without_adding(op, plan_action, t);
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
        goal_idx = m_cnf.get_variable_index_without_adding(goal_var, plan_variable, m_cnf.get_num_timesteps(), goal_val);
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
    LOG_MESSAGE(log_level::info) << "Comparing two assignments. Size of cnf variables and assignment one and two is: "
                                 << m_cnf.get_num_variables() << " " << assignment1.size() << " " << assignment2.size();

    // TODO this will be fixed when i am finished with variable tagging.
    // At this point i hopelfully have a way to output variables uniformly 
    /*
    for (int i = 0; i < m_cnf.get_num_variables(); i++) {
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
    }*/
}

// TODO finish this
std::string get_description_of_ith_variable(int index){

    tagged_variable info = cnf.get_planning_info_for_variable(index);
}