#include "cnf_encoder.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "logging.h"

using namespace planning_logic;

// TODO maybe write multiple encoders for different encodings?
// have to see how advanced binary encoding goes
std::vector<logic_primitive> cnf_encoder::get_logic_primitives(primitive_tag tag, int timestep) {
    update_timesteps(timestep);
    switch (tag) {
        case ini_state:
            return construct_initial_state();
        case goal:
            return construct_goal(timestep);
        case eo_var:
            return construct_exact_one_value(timestep);
        case eo_op:
            return construct_exact_one_action(timestep);
        case mutex:
            return construct_mutex(timestep);
        case precon:
            return construct_precondition(timestep);
        case effect:
            return construct_effect(timestep);
        case frame:
            return construct_frame(timestep);
        case none:
        default:
            return std::vector<logic_primitive>();
    }
}

// The initial state must hold at t = 0.
std::vector<logic_primitive> cnf_encoder::construct_initial_state() {
    std::vector<logic_primitive> result;

    for (int var = 0; var < m_sas_problem.m_variabels.size(); var++) {

        if(m_options.binary_variables){
            int var_size = m_sas_problem.m_variabels[var].m_range;
            int var_val = m_sas_problem.m_initial_state[var];
            std::vector<int> var_encoding = m_symbol_map.get_variable_index_for_var_binary(0, var, var_val, var_size);
            for(int v: var_encoding){
                std::vector<int> new_clause; new_clause.push_back(v);
                result.push_back(logic_primitive(logic_clause, ini_state, 0, new_clause));
            }
        }
        else {
            // unary case
            for (int val = 0; val < m_sas_problem.m_variabels[var].m_range; val++) {
                std::vector<int> new_clause;
                int sym_index = m_symbol_map.get_variable_index(variable_plan_var, 0, var, val);
                if (m_sas_problem.m_initial_state[var] == val) {
                    new_clause.push_back(sym_index);
                } else {
                    new_clause.push_back(-sym_index);
                }

                result.push_back(logic_primitive(logic_clause, ini_state, 0, new_clause));
            }
        }
    }

    return result;
}

// The goal g holds at step n
std::vector<logic_primitive> cnf_encoder::construct_goal(int timestep) {
    std::vector<logic_primitive> result;

    for (int g = 0; g < m_sas_problem.m_goal.size(); g++) {
        std::vector<int> new_clause;
        std::pair<int, int> goal_value = m_sas_problem.m_goal[g];
        int index_var =
            m_symbol_map.get_variable_index(variable_plan_var, timestep, goal_value.first, goal_value.second);

        new_clause.push_back(index_var);
        result.push_back(logic_primitive(logic_clause, goal, timestep, new_clause));
    }

    return result;
}

// At every timestep a sas variable has at least one value
std::vector<logic_primitive> cnf_encoder::construct_exact_one_value(int timestep) {
    std::vector<logic_primitive> result;

    if (m_options.exact_one_constraint) {
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
            std::vector<int> exact_one_should_be_true;
            for (int val = 0; val < m_sas_problem.m_variabels[v].m_range; val++) {
                int index = m_symbol_map.get_variable_index(variable_plan_var, timestep, v, val);
                exact_one_should_be_true.push_back(index);
            }

            result.push_back(logic_primitive(logic_eo, eo_var, timestep, exact_one_should_be_true));
        }
    } else {
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
            std::vector<int> exact_one_should_be_true;
            for (int val = 0; val < m_sas_problem.m_variabels[v].m_range; val++) {
                int index = m_symbol_map.get_variable_index(variable_plan_var, timestep, v, val);
                exact_one_should_be_true.push_back(index);
            }

            // at least one true
            result.push_back(logic_primitive(logic_clause, eo_var, timestep, exact_one_should_be_true));

            // at most one true
            std::vector<std::vector<int>> constrain_clauses =
                generate_at_most_one_constraint(exact_one_should_be_true, variable_h_amost_variable, timestep);
            for (std::vector<int> constraint : constrain_clauses) {
                result.push_back(logic_primitive(logic_clause, eo_var, timestep, constraint));
            }
        }
    }

    return result;
}

// At every step, at least one action is applied
// in the case of parallel plans this is relaxed to only nonconflicting actions can be applied.
// in the binary case this is not needed
std::vector<logic_primitive> cnf_encoder::construct_exact_one_action(int timestep) {
    std::vector<logic_primitive> result;

    if (m_options.binary_encoding) {
        // nothing to do, binary encoding implicit guarantees only one action
        if (m_options.binary_exclude_impossible) {

            // iterate over the indizes that represent imposssible operators
            int num_imp_ops = (1 << m_symbol_map.num_bits_for_binary_var(m_sas_problem.m_operators.size()));
            for(int imp_op = m_sas_problem.m_operators.size(); imp_op < num_imp_ops; imp_op++){
                std::vector<int> op_indizes = m_symbol_map.get_variable_index_for_op_binary(timestep, imp_op);
                std::vector<int> new_clause;
                for(int i: op_indizes){
                    new_clause.push_back(-i);
                }
                result.push_back(logic_primitive(logic_clause, eo_op, timestep, new_clause));
            }
        }
    } else if (m_options.parallel_plan) {
        result = construct_no_conflicting_operators(timestep);
    } else if (m_options.exact_one_constraint) {
        // smarter exact one encoding for dds
        std::vector<int> exact_one_should_be_true;
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int index = m_symbol_map.get_variable_index(variable_plan_op, timestep, op);
            exact_one_should_be_true.push_back(index);
        }
        result.push_back(logic_primitive(logic_eo, eo_op, timestep, exact_one_should_be_true));
    } else {
        // standart exact one encoding
        std::vector<int> exact_one_should_be_true;
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int sym_index = m_symbol_map.get_variable_index(variable_plan_op, timestep, op);
            exact_one_should_be_true.push_back(sym_index);
        }

        // at least one true
        result.push_back(logic_primitive(logic_clause, eo_op, timestep, exact_one_should_be_true));

        // at most one true
        std::vector<std::vector<int>> constrain_clauses =
            generate_at_most_one_constraint(exact_one_should_be_true, variable_h_amost_operator, timestep);
        for (std::vector<int> constraint : constrain_clauses) {
            result.push_back(logic_primitive(logic_clause, eo_op, timestep, constraint));
        }
    }

    return result;
}

// used for parallel plans
std::vector<logic_primitive> cnf_encoder::construct_no_conflicting_operators(int timestep) {
    std::vector<logic_primitive> result;

    for (int op1 = 0; op1 < m_sas_problem.m_operators.size(); op1++) {
        for (int op2 = op1 + 1; op2 < m_sas_problem.m_operators.size(); op2++) {
            if (m_sas_problem.are_operators_conflicting(op1, op2)) {
                std::vector<int> clause;
                clause.push_back(-m_symbol_map.get_variable_index(variable_plan_op, timestep, op1));
                clause.push_back(-m_symbol_map.get_variable_index(variable_plan_op, timestep, op2));

                result.push_back(logic_primitive(logic_clause, eo_op, timestep, clause));
            } else {
                // std::cout << "Found nonconflicting operators: " << m_sas_problem.m_operators[op1].to_string() << " "
                //           << m_sas_problem.m_operators[op2].to_string() << std::endl;
            }
        }
    }
    return result;
}

// If action a is applied at step t, then pre(a) holds at step t.
std::vector<logic_primitive> cnf_encoder::construct_precondition(int timestep) {
    std::vector<logic_primitive> result;

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

            if (m_options.binary_encoding) {
                int index_precondition =
                    m_symbol_map.get_variable_index(variable_plan_var, timestep, effected_var, effected_old_val);
                std::vector<int> op_indizes = m_symbol_map.get_variable_index_for_op_binary(timestep, op);

                std::vector<int> new_clause;
                for (int idx : op_indizes) {
                    new_clause.push_back(-idx);
                }
                new_clause.push_back(index_precondition);

                result.push_back(logic_primitive(logic_clause, precon, timestep, new_clause));

            } else {
                int index_op, index_precondition;
                index_op = m_symbol_map.get_variable_index(variable_plan_op, timestep, op);
                index_precondition =
                    m_symbol_map.get_variable_index(variable_plan_var, timestep, effected_var, effected_old_val);

                std::vector<int> new_clause;
                new_clause.push_back(-index_op);
                new_clause.push_back(index_precondition);

                result.push_back(logic_primitive(logic_clause, precon, timestep, new_clause));
            }
        }
    }
    return result;
}

// If action a is applied at step t, then eff(a) hold at step t + 1.
std::vector<logic_primitive> cnf_encoder::construct_effect(int timestep) {
    std::vector<logic_primitive> result;

    for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
        for (int eff = 0; eff < m_sas_problem.m_operators[op].m_effects.size(); eff++) {
            int effected_var, effected_new_val;
            effected_var = std::get<0>(m_sas_problem.m_operators[op].m_effects[eff]);
            effected_new_val = std::get<2>(m_sas_problem.m_operators[op].m_effects[eff]);

            if (m_options.binary_encoding) {
                int index_effect =
                    m_symbol_map.get_variable_index(variable_plan_var, timestep + 1, effected_var, effected_new_val);
                std::vector<int> op_indizes = m_symbol_map.get_variable_index_for_op_binary(timestep, op);

                std::vector<int> new_clause;
                for (int idx : op_indizes) {
                    new_clause.push_back(-idx);
                }
                new_clause.push_back(index_effect);

                result.push_back(logic_primitive(logic_clause, effect, timestep, new_clause));

            } else {
                int index_op, index_effect;
                index_op = m_symbol_map.get_variable_index(variable_plan_op, timestep, op);
                index_effect =
                    m_symbol_map.get_variable_index(variable_plan_var, timestep + 1, effected_var, effected_new_val);

                std::vector<int> new_clause;
                new_clause.push_back(-index_op);
                new_clause.push_back(index_effect);

                result.push_back(logic_primitive(logic_clause, effect, timestep, new_clause));
            }
        }
    }
    return result;
}

// If atom p changes between steps t and t + 1, an action which supports this
// change must be applied at t:
std::vector<logic_primitive> cnf_encoder::construct_frame(int timestep) {
    std::vector<logic_primitive> result;

    for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {  // for every variable

        // find all possible value changes of a variable
        int domain_size = m_sas_problem.m_variabels[v].m_range;
        for (int val = 0; val < domain_size; val++) {

            // indize of the sat variable of the planning variable
            int index_val_t1, index_val_t2;
            index_val_t1 = m_symbol_map.get_variable_index(variable_plan_var, timestep, v, val);
            index_val_t2 = m_symbol_map.get_variable_index(variable_plan_var, timestep + 1, v, val);

            // find the actions that support this transition.
            // (planning) indizes of the actions that support the change of the variable 
            // to become true or false in the next timestep
            std::vector<int> support_become_false, support_become_true;
            for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
                for (int i = 0; i < m_sas_problem.m_operators[op].m_effects.size(); i++) {
                    std::tuple<int, int, int> op_eff = m_sas_problem.m_operators[op].m_effects[i];
                    int effected_var, val_pre, val_post;
                    effected_var = std::get<0>(op_eff);
                    val_pre = std::get<1>(op_eff);
                    val_post = std::get<2>(op_eff);

                    // check if corrected variable is affected, and it changes to ture or false
                    if ((effected_var == v) && (val_post != val)) {
                        support_become_false.push_back(op);
                    }
                    if ((effected_var == v) && (val_post == val)) {
                        support_become_true.push_back(op);
                    }
                }
            }

            if (m_options.binary_encoding) {
                std::vector<std::vector<int>> dnf_change_to_true, dnf_change_to_false;

                std::vector<int> temp1, temp2, temp3, temp4;
                temp1.push_back(index_val_t1); dnf_change_to_true.push_back(temp1);
                temp2.push_back(-index_val_t2); dnf_change_to_true.push_back(temp2);
                temp3.push_back(-index_val_t1); dnf_change_to_false.push_back(temp3);
                temp4.push_back(index_val_t2); dnf_change_to_false.push_back(temp4);

                for(int op: support_become_true){
                    dnf_change_to_true.push_back(m_symbol_map.get_variable_index_for_op_binary(timestep, op));
                }
                for(int op: support_become_false){
                    dnf_change_to_false.push_back(m_symbol_map.get_variable_index_for_op_binary(timestep, op));
                }

                result.push_back(logic_primitive(logic_dnf, frame, timestep, dnf_change_to_true));
                result.push_back(logic_primitive(logic_dnf, frame, timestep, dnf_change_to_false));

            } else {
                std::vector<int> change_to_true, change_to_false;
                change_to_true.push_back(index_val_t1);
                change_to_true.push_back(-index_val_t2);
                change_to_false.push_back(-index_val_t1);
                change_to_false.push_back(index_val_t2);

                for(int op: support_become_true){
                    change_to_true.push_back(m_symbol_map.get_variable_index(variable_plan_op, timestep, op));
                }
                for(int op: support_become_false){
                    change_to_false.push_back(m_symbol_map.get_variable_index(variable_plan_op, timestep, op));
                }

                result.push_back(logic_primitive(logic_clause, frame, timestep, change_to_true));
                result.push_back(logic_primitive(logic_clause, frame, timestep, change_to_false));
            }   
        }
    }
    return result;
}

// at every timestep it is not allowed for two values in a mutex to be true at the same time
std::vector<logic_primitive> cnf_encoder::construct_mutex(int timestep) {
    if (!m_options.include_mutex) {
        return std::vector<logic_primitive>();
    }
    std::vector<logic_primitive> result;

    for (int m = 0; m < m_sas_problem.m_mutex_groups.size(); m++) {
        std::vector<int> at_most_one_should_be_true;
        for (int i = 0; i < m_sas_problem.m_mutex_groups[m].size(); i++) {
            std::pair<int, int> var_val_pair = m_sas_problem.m_mutex_groups[m][i];
            int index =
                m_symbol_map.get_variable_index(variable_plan_var, timestep, var_val_pair.first, var_val_pair.second);
            at_most_one_should_be_true.push_back(index);
        }

        std::vector<std::vector<int>> constrain_clauses =
            generate_at_most_one_constraint(at_most_one_should_be_true, variable_h_amost_mutex, timestep);
        for (std::vector<int> constraint : constrain_clauses) {
            result.push_back(logic_primitive(logic_clause, mutex, timestep, constraint));
        }
    }

    return result;
}

std::vector<std::vector<int>> cnf_encoder::generate_at_most_one_constraint(std::vector<int> &variables,
                                                                           variable_tag constraint_type, int timestep) {
    if (!m_options.use_ladder_encoding) {
        return generate_at_most_one_constraint_pairwise(variables);
    } else if (m_options.use_ladder_encoding && variables.size() <= 5) {
        return generate_at_most_one_constraint_pairwise(variables);
    } else {
        return generate_at_most_one_constraint_ladder(variables, constraint_type, timestep);
    }
}

std::vector<std::vector<int>> cnf_encoder::generate_at_most_one_constraint_ladder(std::vector<int> &variables,
                                                                                  variable_tag constraint_type,
                                                                                  int timestep) {
    std::vector<std::vector<int>> all_new_clauses;

    for (int i = 0; i < variables.size(); i++) {
        if (i != 0 && i != variables.size() - 1) {  // first and last helper variable dont need this clause
            std::vector<int> new_clause;
            new_clause.push_back(-m_symbol_map.get_variable_index(constraint_type, timestep, i - 1));  // !si-1
            new_clause.push_back(m_symbol_map.get_variable_index(constraint_type, timestep, i));       // si
            all_new_clauses.push_back(new_clause);
        }
        if (i != variables.size() - 1) {  // last variable does not need this implication
            std::vector<int> new_clause;
            new_clause.push_back(-variables[i]);                                                  // !Xi
            new_clause.push_back(m_symbol_map.get_variable_index(constraint_type, timestep, i));  // si
            all_new_clauses.push_back(new_clause);
        }
        if (i != 0) {  // first vaiable does not need this implication
            std::vector<int> new_clause;
            new_clause.push_back(-variables[i]);                                                       // !Xi
            new_clause.push_back(-m_symbol_map.get_variable_index(constraint_type, timestep, i - 1));  // !si-1
            all_new_clauses.push_back(new_clause);
        }
    }

    return all_new_clauses;
}

std::vector<std::vector<int>> cnf_encoder::generate_at_most_one_constraint_pairwise(std::vector<int> &variables) {
    std::vector<std::vector<int>> all_new_clauses;
    for (int i = 0; i < variables.size(); i++) {
        for (int j = i + 1; j < variables.size(); j++) {
            std::vector<int> new_clause;
            new_clause.push_back(-variables[i]);  // !Xi
            new_clause.push_back(-variables[j]);  // !Xj
            all_new_clauses.push_back(new_clause);
        }
    }
    return all_new_clauses;
}

void cnf_encoder::update_timesteps(int timestep){
    m_num_timesteps = timestep > m_num_timesteps ? timestep : m_num_timesteps;
}

// This call depends on the correct symbol map.
std::vector<bool> cnf_encoder::parse_cnf_solution(std::string filepath) {
    LOG_MESSAGE(log_level::info) << "Start parsing assignment file";

    std::ifstream infile(filepath);
    std::string line;
    std::istringstream iss;
    std::vector<bool> assignment(m_symbol_map.get_num_variables(), false);
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
        if (index > m_symbol_map.get_num_variables()) {
            LOG_MESSAGE(log_level::error) << "The assignment values are too big. Symbol map has size "
                                          << m_symbol_map.get_num_variables() << " the assignment has value" << int_val;
        }
        bool bool_val = (int_val > 0) ? true : false;
        assignment[index] = bool_val;

        iss >> int_val;
    }

    return assignment;
}

std::string cnf_encoder::decode_cnf_variable(int index) {
    tagged_variable info = m_symbol_map.get_planning_info_for_variable(index);
    variable_tag tag = std::get<0>(info);
    int v_timestep = std::get<1>(info);
    int v_value = std::get<2>(info);
    int v_index = std::get<3>(info);

    // determin type of
    std::string type;
    std::string name;
    switch (tag) {
        case variable_plan_var:
            type = "plan_variable";
            name = m_sas_problem.m_variabels[v_index].m_name + ": " +
                   m_sas_problem.m_variabels[v_index].m_symbolic_names[v_value];
            break;
        case variable_plan_op:
            type = "plan_action";
            name = m_sas_problem.m_operators[v_index].m_name;
            break;
        case variable_h_amost_variable:
            type = "h_amost_variable";
            break;
        case variable_h_amost_operator:
            type = "h_amost_operator";
            break;
        case variable_h_amost_mutex:
            type = "h_amost_mutex";
            break;
        default:
            type = "unknown";
    }

    return type + " t=" + std::to_string(v_timestep) + " " + name;
}

// TODO make this work with binary encoding
// interprets a solution from minisat. It translates the sat solution to a
// planning problem solution (with some addional debugg information)
// This call depends on the correct symbol map.
void cnf_encoder::decode_cnf_solution(std::vector<bool> &assignment, int num_timesteps) {
    if (assignment.size() == 0) {
        LOG_MESSAGE(log_level::warning) << "Trying to decode an assignment of size 0";
        return;
    }
    for (int t = 0; t <= num_timesteps; t++) {
        // print information about the state
        std::cout << "==========================================" << std::endl;
        std::cout << "Step " << t << " has the following state:" << std::endl;
        for (int v = 0; v < m_sas_problem.m_variabels.size(); v++) {
            for (int val = 0; val < m_sas_problem.m_variabels[v].m_range; val++) {
                int idx = m_symbol_map.get_variable_index_without_adding(variable_plan_var, t, v, val);
                if (assignment[idx]) {
                    std::cout << m_sas_problem.m_variabels[v].m_name << ": "
                              << m_sas_problem.m_variabels[v].m_symbolic_names[val] << std::endl;
                }
            }
        }

        // print information about the operators used
        if (t == num_timesteps) continue;  // dont print an operator for the last state (there is none)
        for (int op = 0; op < m_sas_problem.m_operators.size(); op++) {
            int idx = m_symbol_map.get_variable_index_without_adding(variable_plan_op, t, op);
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
        goal_idx = m_symbol_map.get_variable_index_without_adding(variable_plan_var, num_timesteps, goal_var, goal_val);
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
                                 << m_symbol_map.get_num_variables() << " " << assignment1.size() << " "
                                 << assignment2.size();

    // TODO this will be fixed when i am finished with variable tagging.
    // TODO maybe this will never be fixed?
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