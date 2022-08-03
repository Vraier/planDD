#pragma once

#include <vector>

#include "encoder.h"
#include "graph.h"
#include "logic_primitive.h"
#include "options.h"
#include "plan_to_cnf_map.h"
#include "sas_parser.h"

namespace encoder {

class binary_parallel : public virtual encoder {
   public:
    binary_parallel(option_values &options, sas_problem &problem, graph::undirected_graph &conflict_graph);

    // maps planning variables to cnf variables.
    planning_logic::plan_to_cnf_map m_symbol_map;
    // represents the planning problem
    sas_problem m_sas_problem;

    // constructs the logic primitives according to the tag and timestep
    // will change the symbol map if new variables are created
    std::vector<planning_logic::logic_primitive> get_logic_primitives(planning_logic::primitive_tag tag, int timestep);

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
    // number of colour_classes
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