#pragma once

#include <vector>

#include "graph.h"
#include "logic_primitive.h"
#include "options.h"
#include "plan_to_cnf_map.h"
#include "sas_parser.h"

namespace encoder {

class binary_parallel {
   public:
    binary_parallel(option_values &options, sas_problem &problem, graph::undirected_graph &conflict_graph);

    // maps planning variables to cnf variables.
    planning_logic::plan_to_cnf_map m_symbol_map;
    // represents the planning problem
    sas_problem m_sas_problem;

    // constructs the logic primitives according to the tag and timestep
    // will change the symbol map if new variables are created
    std::vector<planning_logic::logic_primitive> get_logic_primitives(planning_logic::primitive_tag tag, int timestep);

    // parses a cnf solution from minisat into a bool vector
    // assignemnt gives the truth value for the ith variable (index 0 of assignment has no meaningful value)
    // returns an empty vector if the file contains no solution
    std::vector<bool> parse_cnf_solution(std::string filepath);

    // translate the ith cnf variable into the planning context and returns a string description
    // the variable should be greater than 0 and smaller or eaqual to
    // num_variables
    std::string decode_cnf_variable(int cnf_index);
    // interprets the bool vector as a plan for a planning problem
    void decode_cnf_solution(std::vector<bool> &assignment, int num_timesteps);
    // compares two asssignments for a cnf formula and prints the difference in human readable form (for debugging)
    void compare_assignments(std::vector<bool> &assignment1, std::vector<bool> &assignment2);

   private:
    // keeps track of the maximum timestep that was encoded
    int m_num_timesteps;
    // increases num_timesteps to the new maximum
    void update_timesteps(int timestep);

    // holds options for the whole programm. Some are important for the cnf_encoder
    option_values m_options;

    // the edges of the graph represent actions that are not allowed together
    graph::undirected_graph m_action_conflicts;
    // colouring of the complement of the graph above
    std::vector<int> m_colouring;
    int m_num_colours;
    // gives the size of a colour class
    std::vector<int> m_colour_class_size;
    // the id of an action inside a colour class
    std::vector<int> m_group_id;

    // These methods generate all the logic primitives that represent the planning problem
    std::vector<planning_logic::logic_primitive> construct_initial_state();
    std::vector<planning_logic::logic_primitive> construct_goal(int timestep);
    std::vector<planning_logic::logic_primitive> construct_no_impossible_value(int timestep);
    std::vector<planning_logic::logic_primitive> construct_exact_one_action(int timestep);
    std::vector<planning_logic::logic_primitive> construct_mutex(int timestep);
    std::vector<planning_logic::logic_primitive> construct_precondition(int timestep);
    std::vector<planning_logic::logic_primitive> construct_effect(int timestep);
    std::vector<planning_logic::logic_primitive> construct_frame(int timestep);
};
}  // namespace encoder