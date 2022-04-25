#include "sas_parser.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "logging.h"

int sas_parser::start_parsing() {
    LOG_MESSAGE(log_level::info) << "Start Parsing SAS Problem";

    m_sas_problem = sas_problem();  // clear sas_problem just in case someone messed with it

    std::ifstream infile(m_filepath);
    int error = 0;

    error = parse_sas_version(infile);
    if (error != 0) {
        return -1;
    }
    error = parse_sas_metric(infile);
    if (error != 0) {
        return -1;
    }
    error = parse_sas_variables(infile);
    if (error != 0) {
        return -1;
    }
    error = parse_sas_mutex(infile);
    if (error != 0) {
        return -1;
    }
    error = parse_sas_initial_state(infile);
    if (error != 0) {
        return -1;
    }
    error = parse_sas_goal(infile);
    if (error != 0) {
        return -1;
    }
    error = parse_sas_operator(infile);
    if (error != 0) {
        return -1;
    }
    error = parse_sas_axiom(infile);
    if (error != 0) {
        return -1;
    }

    LOG_MESSAGE(log_level::info) << "Finished parsing SAS problem. "
                                 << "The problem has " << m_sas_problem.m_variabels.size() << " variables, "
                                 << m_sas_problem.m_operators.size() << " operators, "
                                 << m_sas_problem.m_initial_state.size() << " is the size of the initial states and "
                                 << m_sas_problem.m_goal.size() << " is the size of the goal";

    return 0;
}

int sas_parser::parse_sas_version(std::ifstream &infile) {
    std::string line;
    std::istringstream iss;

    //LOG_MESSAGE(log_level::info) << "Parsing version";
    std::getline(infile, line);
    if (line != "begin_version") {
        LOG_MESSAGE(log_level::error) << "Expected begin_version";
        return -1;
    }
    std::getline(infile, line);
    if (line != "3") {
        LOG_MESSAGE(log_level::error) << "Not sas version 3";
        return -1;
    }
    std::getline(infile, line);
    if (line != "end_version") {
        LOG_MESSAGE(log_level::error) << "Expected end_verion";
        return -1;
    }

    return 0;
}

int sas_parser::parse_sas_metric(std::ifstream &infile) {
    std::string line;

    //LOG_MESSAGE(log_level::info) << "Parsing metric";
    std::getline(infile, line);
    if (line != "begin_metric") {
        LOG_MESSAGE(log_level::error) << "expected begin_metric" << std::endl;
        return -1;
    }
    std::getline(infile, line);
    if (line != "0") {
        LOG_MESSAGE(log_level::error) << "Not a unit cost problem" << std::endl;
        return -1;
    }
    std::getline(infile, line);
    if (line != "end_metric") {
        LOG_MESSAGE(log_level::error) << "Expected end_metric";
        return -1;
    }

    return 0;
}

int sas_parser::parse_sas_variables(std::ifstream &infile) {
    std::string line;
    std::istringstream iss;
    int num_variables;

    //LOG_MESSAGE(log_level::info) << "Parsing variables" << std::endl;
    std::getline(infile, line);
    iss = std::istringstream(line);
    iss >> num_variables;

    for (int i = 0; i < num_variables; i++) {
        std::string var_name;
        int axiom_layer;
        int var_range;
        std::vector<std::string> symbolic_names;

        std::getline(infile, line);
        if (line != "begin_variable") {
            LOG_MESSAGE(log_level::error) << "Expected begin_variables" << std::endl;
            return -1;
        }
        std::getline(infile, var_name);
        std::getline(infile, line);
        iss = std::istringstream(line);
        iss >> axiom_layer;
        std::getline(infile, line);
        iss = std::istringstream(line);
        iss >> var_range;

        for (int j = 0; j < var_range; j++) {
            std::getline(infile, line);
            symbolic_names.push_back(line);
        }

        std::getline(infile, line);
        if (line != "end_variable") {
            LOG_MESSAGE(log_level::error) << "Expected end_variables" << std::endl;
            return -1;
        }

        variable_info new_info(var_name, var_range, symbolic_names);
        m_sas_problem.m_variabels.push_back(new_info);
    }

    return 0;
}

int sas_parser::parse_sas_mutex(std::ifstream &infile) {
    std::string line;
    std::istringstream iss;
    int num_mutexes;

    //LOG_MESSAGE(log_level::info) << "Parsing mutexes" << std::endl;
    std::getline(infile, line);
    iss = std::istringstream(line);
    iss >> num_mutexes;

    for (int i = 0; i < num_mutexes; i++) {
        int group_size;
        std::vector<std::pair<int, int>> group;

        std::getline(infile, line);
        if (line != "begin_mutex_group") {
            LOG_MESSAGE(log_level::error) << "expected begin_mutex_group" << std::endl;
            return -1;
        }
        std::getline(infile, line);
        iss = std::istringstream(line);
        iss >> group_size;

        for (int j = 0; j < group_size; j++) {
            int var_index, var_value;
            std::getline(infile, line);
            iss = std::istringstream(line);
            iss >> var_index >> var_value;
            group.push_back(std::make_pair(var_index, var_value));
        }

        m_sas_problem.m_mutex_groups.push_back(group);

        std::getline(infile, line);
        if (line != "end_mutex_group") {
            LOG_MESSAGE(log_level::error) << "expected end_mutex_group" << std::endl;
            return -1;
        }
    }

    return 0;
}

int sas_parser::parse_sas_initial_state(std::ifstream &infile) {
    std::string line;
    std::istringstream iss;

    //LOG_MESSAGE(log_level::info) << "Parsing initial state" << std::endl;
    std::getline(infile, line);
    if (line != "begin_state") {
        LOG_MESSAGE(log_level::error) << "Expected begin_state" << std::endl;
        return -1;
    }

    for (int i = 0; i < m_sas_problem.m_variabels.size(); i++) {
        std::getline(infile, line);
        iss = std::istringstream(line);
        int initial_state_value;
        iss >> initial_state_value;
        m_sas_problem.m_initial_state.push_back(initial_state_value);
    }

    std::getline(infile, line);
    if (line != "end_state") {
        LOG_MESSAGE(log_level::error) << "Expected end_state" << std::endl;
        return -1;
    }

    return 0;
}

int sas_parser::parse_sas_goal(std::ifstream &infile) {
    std::string line;
    std::istringstream iss;

    //LOG_MESSAGE(log_level::info) << "Parsing goal" << std::endl;
    std::getline(infile, line);
    if (line != "begin_goal") {
        LOG_MESSAGE(log_level::error) << "Expected begin_goal";
        return -1;
    }
    std::getline(infile, line);
    int goal_size;
    iss = std::istringstream(line);
    iss >> goal_size;

    for (int i = 0; i < goal_size; i++) {
        std::getline(infile, line);
        iss = std::istringstream(line);
        int var_index, var_val;
        iss >> var_index >> var_val;
        m_sas_problem.m_goal.push_back(std::make_pair(var_index, var_val));
    }

    std::getline(infile, line);
    if (line != "end_goal") {
        LOG_MESSAGE(log_level::error) << "Expected end_goal";
        return -1;
    }

    return 0;
}

int sas_parser::parse_sas_operator(std::ifstream &infile) {
    std::string line;
    std::istringstream iss;
    int num_operators;

    //LOG_MESSAGE(log_level::info) << "Parsing operators" << std::endl;
    std::getline(infile, line);
    iss = std::istringstream(line);
    iss >> num_operators;

    for (int i = 0; i < num_operators; i++) {
        std::string operator_name;
        std::vector<std::tuple<int, int, int>> effects;
        int num_prevail_conditions, num_effects;
        std::getline(infile, line);
        if (line != "begin_operator") {
            LOG_MESSAGE(log_level::error) << "Expected begin_operator";
            return -1;
        }
        std::getline(infile, operator_name);

        // parse prevail conditions
        std::getline(infile, line);
        iss = std::istringstream(line);
        iss >> num_prevail_conditions;
        for (int j = 0; j < num_prevail_conditions; j++) {
            int var_index, var_val;
            std::getline(infile, line);
            iss = std::istringstream(line);
            iss >> var_index >> var_val;
            effects.push_back(std::make_tuple(var_index, var_val, var_val));
        }

        // parse effects
        std::getline(infile, line);
        iss = std::istringstream(line);
        iss >> num_effects;
        for (int j = 0; j < num_effects; j++) {
            std::string effect_desc;
            int num_effect_conditions, effect_var, effect_pre, effect_after;
            std::getline(infile, effect_desc);
            iss = std::istringstream(effect_desc);

            // parse effect conditions
            iss >> num_effect_conditions;
            if (num_effect_conditions != 0) {
                LOG_MESSAGE(log_level::error) << num_effect_conditions << " amount of effect conditions" << std::endl;
                return -1;
            }
            for (int k = 0; k < num_effect_conditions; k++) {
                int eff_con_var, eff_con_val;
                iss >> eff_con_var >> eff_con_val;
            }

            // parse rest of effect
            iss >> effect_var >> effect_pre >> effect_after;
            effects.push_back(std::make_tuple(effect_var, effect_pre, effect_after));
        }

        m_sas_problem.m_operators.push_back(operator_info(operator_name, effects));

        std::getline(infile, line);
        iss = std::istringstream(line);
        int operator_cost;
        iss >> operator_cost;
        std::getline(infile, line);
        if (line != "end_operator") {
            LOG_MESSAGE(log_level::error) << "Expected end_operator";
            return -1;
        }
    }

    return 0;
}

int sas_parser::parse_sas_axiom(std::ifstream &infile) {
    std::string line;
    std::istringstream iss;

    std::getline(infile, line);
    iss = std::istringstream(line);
    int num_axioms;
    iss >> num_axioms;
    if (num_axioms != 0) {
        LOG_MESSAGE(log_level::error) << num_axioms << " amount of axioms" << std::endl;
        return -1;
    }

    return 0;
}

std::string variable_info::to_string() {
    std::stringstream ss;
    ss << "Var: " << this->m_name << " " << this->m_range;
    for (int i = 0; i < this->m_range; i++) {
        ss << " " << this->m_symbolic_names[i];
    }
    return ss.str();
}

std::string operator_info::to_string() {
    std::stringstream ss;
    ss << "Op: " << this->m_name;
    for (int i = 0; i < this->m_effects.size(); i++) {
        ss << " " << std::get<0>(this->m_effects[i]) << " " << std::get<1>(this->m_effects[i]) << " "
           << std::get<2>(this->m_effects[i]);
    }
    return ss.str();
}