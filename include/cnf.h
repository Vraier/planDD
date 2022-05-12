#ifndef CNF_H
#define CNF_H

#include <map>
#include <string>
#include <vector>

namespace planning_cnf {

enum clause_tag {
    initial_state,  // revelvant for timestep 0
    goal,           // only for tiemestep n
    at_least_var,   // for timestep 0..n
    at_most_var,    // 0..n
    at_least_op,    // 0..n-1
    at_most_op,     // ipex:mc
    mutex,          // igxpc:m -> i0gn m0m1m2..mn p0..pn c0..cn
    precondition,   // igmx:cp -> i0gn m0m1..mn c0p0 c1p1..cnpn
    effect,
    changing_atoms,
    none_clause,
};

enum variable_tag {
    plan_variable,
    plan_action,
    h_amost_variable,
    h_amost_operator,
    h_amost_mutex,
    none_variable,
};

typedef std::vector<int> clause;

// definition of clause, tag, timestep
typedef std::tuple<clause, clause_tag, int> tagged_clause;
// index of the variable into planning problem, tag, timestep, value (in case of planning variable)
typedef std::tuple<int, variable_tag, int, int> tagged_variable;

class cnf {
   private:
    /* data */
    int m_num_timesteps;

    // sorted clauses by order of creation
    std::vector<tagged_clause> clauses;

    std::vector<clause> m_clauses;
    std::vector<clause_tag> m_tags;
    std::vector<int> m_timesteps;

   public:   
    // maps information about planning variable to variable index of cnf formula
    // are used by the variable ordering algorithm
    std::map<tagged_variable, int> m_variable_map;
    std::map<int, tagged_variable> m_inverse_variable_map;

    cnf(int num_timesteps);
    ~cnf();

    // adds a clause to the cnf. the tag indicates which type of clause this is
    // if the type is initial_state, the timestep will alway be zero, for the goal it will always be n
    void add_clause(clause clause, clause_tag tag, int timestep);

    // These methos get the information about a variable/action from a planning problem and return 
    // the variable index into the cnf formula
    // They also add is to the pool of variables
    // This method is used for planning variables: var_index, tag, timestep, value
    int get_variable_index(int var_index, variable_tag tag, int timestep, int value);
    // This method is used for planning operators and helper variables: var_index, tag, timestep
    int get_variable_index(int var_index, variable_tag tag, int timestep);

    // same as above but does not add if variable does not exists. Returns -1 in this case
    int get_variable_index_without_adding(int var_index, variable_tag tag, int timestep, int value);
    int get_variable_index_without_adding(int var_index, variable_tag tag, int timestep);

    // inverse of the methods above
    tagged_variable get_planning_info_for_variable(int index);

    // returns the ith clause, tar, timestep in the order they were added
    clause get_clause(int i);
    clause_tag get_tag(int i);
    int get_timestep(int i);

    int get_num_variables();
    int get_num_clauses();
    int get_num_timesteps();

    // writes the cnf to file in standart format
    void write_to_file(std::string filepath);
};

}  // namespace planning_cnf

#endif