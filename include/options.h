#pragma once

#include <boost/program_options.hpp>
#include <iostream>

// TODO make '_' to '-' and think about better names
struct option_values {
    // Files
    std::string sas_file, ass_file, cnf_file;
    // Program modes
    bool encode_cnf, cnf_to_bdd, conflict_graph, build_bdd, build_sdd, single_minisat, count_minisat, hack_debug;

    int timesteps;
    // DD building parameters
    std::string build_order, variable_order;
    // effects the building algorithm
    bool layer, layer_bi, layer_expo, linear;
    // reverses the order of primitives (only for linear non incremental)
    bool reverse_order;
    // effects the encoding
    bool include_mutex, use_ladder_encoding, group_pre_eff, exact_one_constraint, parallel_plan, binary_encoding,
        binary_variables, binary_exclude_impossible, binary_parallel;
    // effects the variable ordering
    bool no_reordering, goal_variables_first, initial_state_variables_first;
    // effects variable_grouping
    bool group_variables, group_variables_small, group_actions;
    // DD layer building
    bool share_foundations, use_layer_permutation, reverse_layer_building;

    // variable ordering with custom, force or combined
    bool var_order_custom, var_order_force, var_order_custom_force;
};

/**
 * Parses the options from the command line into the option_values struct.
 * Can check for validity of the options.
 * Can print information about the useage of the programm.
 */
class option_parser {
   private:
    boost::program_options::options_description m_option_desc;
    boost::program_options::variables_map m_argument_map;

   public:
    option_values m_values;
    option_parser(/* args */) : m_option_desc("All Options") {}

    void parse_command_line(int argc, char *argv[]);
    bool check_validity();
    void print_help();
    void print_variable_map();
};