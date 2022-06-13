#pragma once

#include <boost/program_options.hpp>
#include <iostream>

// TODO make _ to -
struct option_values {
    // Files
    std::string sas_file, ass_file, cnf_file;
    // Program modes
    bool encode_cnf, cnf_to_bdd, build_bdd, build_bdd_by_layer, build_sdd, single_minisat, count_minisat, hack_debug;
    
    int timesteps;
    // DD building parameters
    std::string build_order, variable_order;
    bool reverse_order;
    bool include_mutex, use_ladder_encoding, exact_one_constraint, goal_variables_first, initial_state_variables_first;
    // DD layer building
    bool layer_on_the_fly, use_layer_permutation;
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