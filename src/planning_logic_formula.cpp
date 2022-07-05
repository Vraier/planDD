#include "planning_logic_formula.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "logging.h"

namespace planning_logic {



int formula::get_num_variables() { return m_symbol_map.get_num_variables(); }
int formula::get_num_clauses() { return m_num_clauses; }
int formula::get_num_constraints() { return m_num_eo_constraints; }
int formula::get_num_timesteps() { return m_num_timesteps; }

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

std::tuple<int, int, std::vector<std::vector<int>>> formula::parse_cnf_file_to_clauses(std::string file_path) {
    LOG_MESSAGE(log_level::info) << "Starting to parse cnf file to clauses";

    std::vector<std::vector<int>> all_clauses;
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
        std::vector<int> new_clause;
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

}  // namespace planning_logic