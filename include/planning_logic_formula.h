#pragma once

#include <map>
#include <string>
#include <vector>

/* TODO: i really have to refactor the cnf code:
 * rename it to someting like logic containter
 * make variable and clause storing more uniform and eliminate the categorize clause/variable functions
 * make names of enums more consistent
 */
namespace planning_logic {

enum variable_tag {
    variable_plan_var,
    variable_plan_op,
    variable_h_amost_variable,
    variable_h_amost_operator,
    variable_h_amost_mutex,
    variable_none,
};

enum clause_tag {
    clause_ini_state,
    clause_goal,
    clause_al_var, // at least one var is true
    clause_am_var, // at most one var is true
    clause_al_op,
    clause_am_op,
    clause_mutex,
    clause_precon,
    clause_effect,
    clause_frame,
    clause_none,
};

// exactly one variable is true
enum eo_constraint_tag {
    eo_var,
    eo_op,
    eo_none,
};

enum logic_primitive_type {
    logic_clause,
    logic_eo,
};

// logic primitive can be a clause or an exactly one constraint
typedef std::vector<int> logic_primitive;
typedef std::vector<int> clause;
typedef std::vector<int> eo_constraint;

// definition of clause, tag, timestep
typedef std::tuple<clause, clause_tag, int> tagged_clause;
// definition of constraint, tag, timestep
typedef std::tuple<eo_constraint, eo_constraint_tag, int> tagged_constraint;
// index of the variable into planning problem, tag, timestep, value (in case of planning variable)
typedef std::tuple<int, variable_tag, int, int> tagged_variable;

class cnf {
   private:
    /* data */
    int m_num_timesteps;

    // sorted clauses by order of creation
    std::vector<tagged_clause> m_tagged_clauses;

    std::vector<tagged_constraint> m_constraints;

   public:
    // maps information about planning variable to variable index of cnf formula
    // are used by the variable ordering algorithm
    std::map<tagged_variable, int> m_variable_map;
    std::map<int, tagged_variable> m_inverse_variable_map;

    cnf(int num_timesteps);
    cnf(std::string file_path);
    ~cnf();

    // adds a clause to the cnf. the tag indicates which type of clause this is
    // if the type is initial_state, the timestep will alway be zero, for the goal it will always be n
    void add_clause(clause clause, clause_tag tag, int timestep);

    // adds a constraint that ensures that exactly one variable from constraints is true
    void add_exact_one_constraint(eo_constraint constraint, eo_constraint_tag tag, int timestep);

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
    clause_tag get_clause_tag(int i);
    int get_clause_timestep(int i);
    // the same but for the exact one variable is true constraints
    eo_constraint get_constraint(int i);
    eo_constraint_tag get_constraint_tag(int i);
    int get_constraint_timestep(int i);

    int get_num_variables();
    int get_num_clauses();
    int get_num_constraints();
    int get_num_timesteps();

    // writes the cnf to file in standart format
    void write_to_file(std::string filepath);
    // take a cnf file and tries to parse it into a set of clauses (kind of inverse for above)
    // return num_variable, num_clauses, and the clauses
    static std::tuple<int, int, std::vector<clause>> parse_cnf_file_to_clauses(std::string file_path);
};

}  // namespace planning_logic