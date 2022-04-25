#include "cnf.h"

using namespace planning_cnf;

cnf::cnf(int num_variables, int num_timestpes){
    m_num_variables = num_variables;
    m_num_timesteps = num_timestpes;
}

cnf::~cnf(){}

void cnf::add_clause(std::vector<int> clause, tag tag, int timestep) {
    m_clauses.push_back(clause);
    m_tags.push_back(tag);
    m_timesteps.push_back(timestep);
}

std::vector<int> cnf::get_clause(int i) { return m_clauses[i]; }

tag cnf::get_tag(int i) { return m_tags[i]; }

int cnf::get_timestep(int i) { return m_timesteps[i]; }

int cnf::get_num_variables() { return m_num_variables; }
int cnf::get_num_clauses() { return m_clauses.size(); }
int cnf::get_num_timesteps() { return m_num_timesteps; }