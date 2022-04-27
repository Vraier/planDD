#ifndef CNF_H
#define CNF_H

#include <map>
#include <string>
#include <vector>

namespace planning_cnf {

typedef std::vector<int> clause;

enum tag {
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
    none,
};

class cnf {
   private:
    /* data */
    int m_num_variables, m_num_timesteps;

    // sort the clauses by order of creation
    std::vector<clause> m_clauses;
    std::vector<tag> m_tags;
    std::vector<int> m_timesteps;

    // sortes the clauses by tag and then by timesteps
    std::map<tag, std::vector<std::vector<clause>>> m_sorted_clauses;

   public:
    cnf(int num_variables, int num_timestpes);
    ~cnf();

    // adds a clause to the cnf. the tag indicates which type of clause this is
    // if the type is initial_state, the timestep will alway be zero, for the goal it will always be n
    void add_clause(clause clause, tag tag, int timestep);

    // returns the ith clause, tar, timestep in the order they were added
    clause get_clause(int i);
    tag get_tag(int i);
    int get_timestep(int i);

    int get_num_variables();
    int get_num_clauses();
    int get_num_timesteps();
};

}  // namespace planning_cnf

#endif