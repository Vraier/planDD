#include "cnf.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "logging.h"

using namespace planning_cnf;

cnf::cnf(int num_timestpes){
    m_num_timesteps = num_timestpes;
}

cnf::~cnf(){}

void cnf::add_clause(std::vector<int> clause, clause_tag tag, int timestep) {
    m_clauses.push_back(clause);
    m_tags.push_back(tag);
    m_timesteps.push_back(timestep);
}

int cnf::get_variable_index(int var_index, variable_tag tag, int timestep, int value){
    tagged_variable var = std::make_tuple(var_index, tag, timestep, value);
    if (m_variable_map.find(var) == m_variable_map.end()) {
        int size = m_variable_map.size();
        m_variable_map[var] = size + 1;
    }

    // also add information to the inverted variable map
    m_inverse_variable_map[m_variable_map[var]] = var;
    return m_variable_map[var];
}

int cnf::get_variable_index_without_adding(int var_index, variable_tag tag, int timestep, int value){
    tagged_variable var = std::make_tuple(var_index, tag, timestep, value);
    if (m_variable_map.find(var) == m_variable_map.end()) {
        return -1;
    }
    return m_variable_map[var];
}

int cnf::get_variable_index(int var_index, variable_tag tag, int timestep){
    return get_variable_index(var_index, tag, timestep, 0);
}

int cnf::get_variable_index_without_adding(int var_index, variable_tag tag, int timestep){
    return get_variable_index_without_adding(var_index, tag, timestep, 0);
}

// TODO implement
tagged_variable cnf::get_planning_info_for_variable(int index){
    return m_inverse_variable_map[index];
}

std::vector<int> cnf::get_clause(int i) { return m_clauses[i]; }
clause_tag cnf::get_tag(int i) { return m_tags[i]; }
int cnf::get_timestep(int i) { return m_timesteps[i]; }

int cnf::get_num_variables() { return m_variable_map.size(); }
int cnf::get_num_clauses() { return m_clauses.size(); }
int cnf::get_num_timesteps() { return m_num_timesteps; }

void cnf::write_to_file(std::string filepath) {
    std::fstream file_out;
    file_out.open(filepath, std::ios_base::out);

    file_out << "p cnf " << get_num_variables() << " " << get_num_clauses() << std::endl;
    for (int i = 0; i < get_num_clauses(); i++) {
        std::vector<int> clause = get_clause(i);
        for (int l : clause) {
            file_out << l << " ";
        }
        file_out << "0" << std::endl;
    }
}

std::tuple<int, int, std::vector<clause>> cnf::parse_cnf_file_to_clauses(std::string file_path){

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
    iss >> s_p; iss >> s_cnf; 
    iss >> num_variables; iss >> num_clauses;

    LOG_MESSAGE(log_level::debug) << s_p << " " << s_cnf << " " << num_variables << " " << num_clauses;

    while(std::getline(infile, line)){
        iss = std::istringstream(line);
        clause new_clause;
        int var;
        iss >> var;
        while(var != 0) {
            new_clause.push_back(var);
            iss >> var;
        }
        all_clauses.push_back(new_clause);
    }

    return std::make_tuple(num_variables, num_clauses, all_clauses);
}