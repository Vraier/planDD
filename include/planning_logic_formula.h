#pragma once

#include <map>
#include <string>
#include <vector>

#include "logic_primitive.h"
#include "plan_to_cnf_map.h"

// TODO: maybe make tagged var, clause, etc to struct
namespace planning_logic {

// primitive tag, timestep
typedef std::tuple<primitive_tag, int> tagged_primitive;

class formula {
   private:
    /* data */
    int m_num_timesteps;
    int m_num_clauses;
    int m_num_eo_constraints;

    plan_to_cnf_map m_symbol_map;

   public:
    // maps tag and timestep to all clauses that belong to this pair
    std::map<tagged_primitive, std::vector<logic_primitive>> m_primitive_map;

    formula(plan_to_cnf_map &symbol_map, int num_timesteps)
        : m_num_timesteps(num_timesteps), m_num_clauses(0), m_num_eo_constraints(0), m_symbol_map(symbol_map){};
    formula(std::string file_path);

    // adds a clause to the cnf. the tag indicates which type of clause this is
    // if the type is initial_state, the timestep will alway be zero, for the goal it will always be n
    // adds a constraint that ensures that exactly one variable from constraints is true
    void add_primitive(logic_primitive primitive);

    int get_num_variables();
    int get_num_clauses();
    int get_num_constraints();
    int get_num_timesteps();

    // calculate a mapping that advances the timestep of every variable bt t_diff
    std::vector<int> calculate_permutation_by_timesteps(int t_diff);

    // catgeorizes the clauses of a planning cnf into buckets.
    // each tag gets a bucket. Each buckets is hase one subbucket for each timestep
    void print_info_about_number_of_logic_primitives(planning_logic::formula &cnf);

    // writes the cnf to file in standart format
    void write_to_file(std::string filepath);
    // take a cnf file and tries to parse it into a set of clauses (kind of inverse for above)
    // return num_variable, num_clauses, and the clauses
    static std::tuple<int, int, std::vector<std::vector<int>>> parse_cnf_file_to_clauses(std::string file_path);
};

}  // namespace planning_logic