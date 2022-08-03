#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "encoder.h"
#include "planning_logic_formula.h"
#include "options.h"
#include "sas_parser.h"
#include "logic_primitive.h"
#include "plan_to_cnf_map.h"

namespace encoder {

// TODO: rename to encoder basic
class cnf_encoder : public virtual encoder {
   public:
    cnf_encoder(option_values &options, sas_problem &problem) : m_symbol_map(problem.m_operators.size()), m_sas_problem(problem), m_options(options) {}

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

    // These methods generate all the logic primitives that represent the planning problem
    std::vector<planning_logic::logic_primitive> construct_initial_state();
    std::vector<planning_logic::logic_primitive> construct_goal(int timestep);
    std::vector<planning_logic::logic_primitive> construct_exact_one_value(int timestep);
    std::vector<planning_logic::logic_primitive> construct_exact_one_action(int timestep);
    std::vector<planning_logic::logic_primitive> construct_mutex(int timestep);
    std::vector<planning_logic::logic_primitive> construct_precondition(int timestep);
    std::vector<planning_logic::logic_primitive> construct_effect(int timestep);
    std::vector<planning_logic::logic_primitive> construct_frame(int timestep);

    // generates a set of clauses that gurarantee that at most on of the variables is true
    // can insert helper variables in the symbol map, depending wich at most one type is chosen
    std::vector<std::vector<int>> generate_at_most_one_constraint(std::vector<int> &variables,
                                                                  planning_logic::variable_tag constraint_type,
                                                                  int timestep);
    // Should only be called once per timestep and constraint_type
    // otherwise the helper variables will get reused
    std::vector<std::vector<int>> generate_at_most_one_constraint_ladder(std::vector<int> &variables,
                                                                         planning_logic::variable_tag constraint_type,
                                                                         int timestep);
    // pairwise generates no additional helper variables
    std::vector<std::vector<int>> generate_at_most_one_constraint_pairwise(std::vector<int> &variables);

    std::vector<planning_logic::logic_primitive> construct_no_conflicting_operators(int timestep);
};
}