#include "planning_logic_formula.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "logging.h"

// TODO rename to something like planning logic formula
using namespace planning_logic;

formula::formula(int num_timestpes) {
    m_num_timesteps = num_timestpes;
    m_num_clauses = 0;
    m_num_eo_constraints = 0;
}

formula::~formula() {}

void formula::add_clause(std::vector<int> clause, clause_tag tag, int timestep) {
    m_clause_map[std::make_tuple(tag, timestep)].push_back(clause);
    m_num_clauses++;
}

void formula::add_exact_one_constraint(eo_constraint constraint, eo_constraint_tag tag, int timestep) {
    m_eo_constraint_map[std::make_tuple(tag, timestep)].push_back(constraint);
    m_num_eo_constraints++;
}

int formula::get_variable_index(variable_tag tag, int timestep, int var_index, int value) {
    tagged_variable var = std::make_tuple(tag, timestep, var_index, value);
    if (m_variable_map.find(var) == m_variable_map.end()) {
        int size = m_variable_map.size();
        m_variable_map[var] = size + 1;
    }

    // also add information to the inverted variable map
    m_inverse_variable_map[m_variable_map[var]] = var;
    return m_variable_map[var];
}

int formula::get_variable_index_without_adding(variable_tag tag, int timestep, int var_index, int value) {
    tagged_variable var = std::make_tuple(tag, timestep, var_index, value);
    if (m_variable_map.find(var) == m_variable_map.end()) {
        return -1;
    }
    return m_variable_map[var];
}

int formula::get_variable_index(variable_tag tag, int timestep, int var_index) {
    return get_variable_index(tag, timestep, var_index, 0);
}

int formula::get_variable_index_without_adding(variable_tag tag, int timestep, int var_index) {
    return get_variable_index_without_adding(tag, timestep, var_index, 0);
}

tagged_variable formula::get_planning_info_for_variable(int index) {
    if (m_inverse_variable_map.find(index) == m_inverse_variable_map.end()) {
        return std::make_tuple(variable_none, -1, -1, -1);
    }
    return m_inverse_variable_map[index];
}

int formula::get_num_variables() { return m_variable_map.size(); }
int formula::get_num_clauses() { return m_num_clauses; }
int formula::get_num_constraints() { return m_num_eo_constraints; }
int formula::get_num_timesteps() { return m_num_timesteps; }

std::vector<int> formula::calculate_permutation_by_timesteps(int t_diff) {
    LOG_MESSAGE(log_level::info) << "Calculating permutation of size " << m_variable_map.size() + 1
                                 << " for t_diff=" << t_diff;

    std::vector<int> from_to_index(m_variable_map.size() + 1);
    from_to_index[0] = 0;  // dummy variable does not get permuted

    // calculate the mapping
    for (std::map<planning_logic::tagged_variable, int>::iterator iter = m_variable_map.begin();
         iter != m_variable_map.end(); ++iter) {
        planning_logic::tagged_variable tagged_var = iter->first;
        planning_logic::variable_tag tag = std::get<0>(tagged_var);
        int t = std::get<1>(tagged_var);
        int t_to;

        // also allow negative t_diffs
        if (tag == planning_logic::variable_plan_var) {
            // variables are relevant for t+1 timesteps
            int modulus = m_num_timesteps + 1;
            t_to = (modulus + t + t_diff) % modulus;
        }
        if (tag == planning_logic::variable_plan_op) {
            // operators are only relevant for t timesteps
            int modulus = m_num_timesteps;
            t_to = (modulus + t + t_diff) % modulus;
        }

        planning_logic::tagged_variable tagged_var_to = tagged_var;
        std::get<1>(tagged_var_to) = t_to;
        from_to_index[m_variable_map[tagged_var]] = m_variable_map[tagged_var_to];
    }

    return from_to_index;
}

// TODO: fix
void formula::write_to_file(std::string filepath) {
    std::fstream file_out;
    file_out.open(filepath, std::ios_base::out);

    file_out << "p cnf " << get_num_variables() << " " << get_num_clauses() << std::endl;
    for (int i = 0; i < get_num_clauses(); i++) {
        // std::vector<int> clause = get_clause(i);
        // for (int l : clause) {
        //     file_out << l << " ";
        // }
        file_out << "0" << std::endl;
    }
}

std::tuple<int, int, std::vector<clause>> formula::parse_cnf_file_to_clauses(std::string file_path) {
    LOG_MESSAGE(log_level::info) << "Starting to parse cnf file to clauses";

    std::vector<clause> all_clauses;
    std::ifstream infile(file_path);
    std::string line;
    std::istringstream iss;

    // parse fist line
    std::getline(infile, line);
    iss = std::istringstream(line);
    std::string s_p, s_cnf;
    int num_variables, num_clauses;
    iss >> s_p;
    iss >> s_cnf;
    iss >> num_variables;
    iss >> num_clauses;

    LOG_MESSAGE(log_level::debug) << s_p << " " << s_cnf << " " << num_variables << " " << num_clauses;

    while (std::getline(infile, line)) {
        iss = std::istringstream(line);
        clause new_clause;
        int var;
        iss >> var;
        while (var != 0) {
            new_clause.push_back(var);
            iss >> var;
        }
        all_clauses.push_back(new_clause);
    }

    return std::make_tuple(num_variables, num_clauses, all_clauses);
}