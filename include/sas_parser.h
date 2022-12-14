#pragma once

#include "graph.h"

#include <string>
#include <vector>
#include <sstream>
#include <map>

class variable_info {
   public:
    std::string m_name;
    int m_range;
    std::vector<std::string> m_symbolic_names;

    variable_info(std::string name, int range, std::vector<std::string> symbolic_name)
        : m_name(name), m_range(range), m_symbolic_names(symbolic_name) {}
    std::string to_string();
};

class operator_info {
   public:
    std::string m_name;
    std::vector<std::tuple<int, int, int>>
        m_effects;  // id of effected variable, old value (precondition), new value (effect)

    operator_info(std::string name, std::vector<std::tuple<int, int, int>> effects)
        : m_name(name), m_effects(effects) {}
    std::string to_string();
};

class sas_problem {
   private:
    // checks if two set of variables are consistent (set = set or var_idx, val pairs)
    // a set if consistent if it doe not conatain v=a and v=b with a!=b
    bool are_variables_consistent(std::vector<std::pair<int, int>> &set1, std::vector<std::pair<int, int>> &set2);

   public:
    std::vector<variable_info> m_variabels;
    std::vector<operator_info> m_operators;
    // each vector represents a mutex group.
    // A mutex group consists of variable value pair
    // it is not allowd for two pairs in a group to hold at the same time
    std::vector<std::vector<std::pair<int, int>>> m_mutex_groups;

    std::vector<int> m_initial_state;
    std::vector<std::pair<int, int>> m_goal;

    // checks if two operators are conflicting
    // the are conflicting if one of p1&p2, e1&e2, e1&p2, e2&p1 is not consistent
    bool are_operators_conflicting(int op_idx_1, int op_idx_2);

    graph::undirected_graph construct_action_conflic_graph();
    graph::undirected_graph construct_complement_action_conflic_graph();
};

// returns the legth of the plan found by a fd_run
int get_plan_length(std::string file_path);

class sas_parser {
   public:
    const std::string m_filepath;
    sas_problem m_sas_problem;

    sas_parser(std::string filepath) : m_filepath(filepath) {}

    // returns -1 on error and 0 else
    // fast downward sas file fomat is defined here: https://www.fast-downward.org/TranslatorOutputFormat
    int start_parsing();

    int parse_sas_version(std::ifstream& infile);
    int parse_sas_metric(std::ifstream& infile);
    int parse_sas_variables(std::ifstream& infile);
    int parse_sas_mutex(std::ifstream& infile);
    int parse_sas_initial_state(std::ifstream& infile);
    int parse_sas_goal(std::ifstream& infile);
    int parse_sas_operator(std::ifstream& infile);
    int parse_sas_axiom(std::ifstream& infile);

   private:
    // these functions get a filestream and modify the internal sas_problem
    // the have to be called in the correct order
    // the function start_parsing() handels these methods
    int parse_sas_file(std::string file_path);
};