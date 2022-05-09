#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "cnf.h"
#include "options.h"
#include "sas_parser.h"

class cnf_encoder {
   public:
    cnf_encoder(option_values &options, sas_problem &problem) : m_options(options), m_sas_problem(problem), m_cnf(0) {}
    planning_cnf::cnf encode_cnf(int timesteps);

    // parses a cnf solution from minisat into a bool vector
    // assignemnt gives the truth value for the ith variable (index 0 of assignment has no meaningful value)
    // returns an empty vector if the file contains no solution
    std::vector<bool> parse_cnf_solution(std::string filepath);

    // translate the ith cnf variable into the planning context and returns a string description
    // the variable should be greater than 0 and smaller or eaqual to
    // num_variables
    std::string decode_cnf_variable(int cnf_index);
    // interprets the bool vector as a plan for a planning problem
    void decode_cnf_solution(std::vector<bool> &assignment);
    // compares two asssignments for a cnf formula and prints the difference in human readable form (for debugging)
    void compare_assignments(std::vector<bool> &assignment1, std::vector<bool> &assignment2);


   private:
    // holds options for the whole programm. Some are important for the cnf_encoder
    option_values m_options;

    // represents the planning problem
    sas_problem m_sas_problem;

    // vector of clauses. Each clause is represented by a vector of literals
    // the values fo the literals refer to the symbol_map
    planning_cnf::cnf m_cnf;

    // TODO
    // All three: sas_problem, symbol map and cnf are needed to produce human readable output.
    // They depend on each other. I have to make sure that no inconsistencies occur between them in this class

    // generates a set of clauses that gurarantee that at most on of the variables is true
    // can insert helper variables in the symbol map, depending wich at most one type is chosen
    std::vector<std::vector<int>> generate_at_most_one_constraint(std::vector<int> &variables,
                                                                  planning_cnf::variable_tag constraint_type,
                                                                  int timestep);
    // Should only be called once per timestep and constraint_type
    // otherwise the helper variables will get reused
    std::vector<std::vector<int>> generate_at_most_one_constraint_ladder(std::vector<int> &variables,
                                                                         planning_cnf::variable_tag constraint_type,
                                                                         int timestep);
    // pairwise generates no additional helper variables
    std::vector<std::vector<int>> generate_at_most_one_constraint_pairwise(std::vector<int> &variables);

    // These methos generate all the clauses that represent the planning problem
    void construct_initial_state_clauses();
    void construct_goal_holds_clauses(int timesteps);
    void construct_at_least_on_value_clause(int timesteps);
    void construct_at_most_on_value_clause(int timesteps);
    void construct_at_least_one_action_clauses(int timesteps);
    void construct_at_most_one_action_clauses(int timesteps);
    void construct_mutex_clauses(int timesteps);
    void construct_precondition_clauses(int timesteps);
    void construct_effect_clauses(int timesteps);
    void construct_changing_atom_implies_action_clauses(int timesteps);
};