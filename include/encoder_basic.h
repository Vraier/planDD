#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "encoder_abstract.h"

namespace encoder {

class encoder_basic : public virtual encoder_abstract {
   public:
    encoder_basic(option_values &options, sas_problem &problem)
        : encoder_abstract(options, problem, problem.m_operators.size()) {}

    std::vector<planning_logic::logic_primitive> get_logic_primitives(planning_logic::primitive_tag tag, int timestep);
    
    // returns the number of variables that occur in t timesteps
    // these include the var and operator variables, but not the operator variables for the last timestep
    int num_variables_in_t_timesteps(int t);

    // reserves the indizes for the variables of the given timestep in the symbol_map
    // uses the default variable order
    void create_variables_for_timestep(int t);

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
}  // namespace encoder