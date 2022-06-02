#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "dd_buildable.h"
#include "planning_logic_formula.h"

extern "C" {
#include "cudd.h"
#include "util.h"
}

// code wont compile if i #include <iostream> after util.h

class bdd_container : public virtual dd_buildable {
   private:
    /* data */
    DdManager *m_bdd_manager;
    DdNode *m_root_node;
    int m_num_variables;

    //Functions for building the bdd timestep by timestep
    // builds the bdd for the precondition/effect and frameclauses of timestep 0
    void build_bdd_for_single_step(planning_logic::formula &clauses);
    // returns a node for the main bdd_manager that represents the bdd for a single timestep 
    DdNode* copy_bdd_to_other_container(bdd_container &copy_to);
    // swaps the variables from two timesteps
    void swap_variables_to_other_timestep(int timesetp_from, int timestep_to);
    // returns the variable order for timesetep 0 as used by the sub manager
    // maps var idx -> layer in bdd (there are no gaps in the layers)
    std::vector<int> get_variable_order_for_single_step(std::map<planning_logic::tagged_variable, int> &variable_map);
    // extends the variable order for timestep 0 to all timesteps
    std::vector<int> extend_variable_order_to_all_steps(std::map<planning_logic::tagged_variable, int> &variable_map, int timesteps, std::vector<int> &single_step_order);

   public:
    // constructor for bdd manager. The number of used variables should be clear from the start
    // this is important for counting the number of solutions and variable ordering
    bdd_container(int num_variables);
    virtual ~bdd_container();

    // adds a clause to the root node
    virtual void conjoin_clause(std::vector<int> &clause);
    // adds an exact one constraint to the root node
    virtual void add_exactly_one_constraint(std::vector<int> &variables);

    // reorders the variables with the reorder_shift_converge method
    void reduce_heap();

    // The i-th entry of the permutation array contains the index of the variable that should be brought to the i-th level
    void set_variable_order(std::vector<int> &variable_order);
    // returns the permutation of the variable order
    // the i-th entry contains the the level in the BDD in which the ith variable resides in
    std::vector<int> get_variable_order();

    // Wirtes all the information about the CUDD manager to std::out
    void print_bdd_info();
    // returns a short string with a few information about the current stae of CUDD
    // can be used to evaluate the progress of CUDD during the execution of the program
    virtual std::string get_short_statistics();
    // writes the bdd to a file in dot format
    void write_bdd_to_dot_file(std::string filename);
    
    // function is purely for debugging purpose. allows entry point to bdd manager
    void hack_back_rocket_method();
};