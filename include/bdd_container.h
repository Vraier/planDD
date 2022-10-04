#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "dd_buildable.h"
#include "logging.h"
#include "planning_logic_formula.h"

extern "C" {
#include "mtr.h"
#include "util.h"
#include "cudd.h"
}

// code wont compile if i #include <iostream> after util.h

// TODO remove m_num_variables
class bdd_container : public virtual dd_buildable {
   private:
    /* data */
    DdManager *m_bdd_manager;
    // maps bdd index to the root nodes of the bdd
    // the oth entry is always the main bdd
    std::vector<DdNode *> m_root_nodes;

   public:
    // constructor for bdd manager. The number of used variables should be clear from the start
    // this is important for counting the number of solutions and variable ordering
    bdd_container(int num_bdds);
    ~bdd_container();
    // deletes all previous dds and creates num_dds new dds
    void set_num_dds(int num_dds);

    // check if the bdd is the zero node
    bool is_constant_false(int bdd_index);

    // frees all the nodes under this bdd
    void clear_dd(int dd_index = 0);
    // adds a clause to the root node
    void add_clause_primitive(std::vector<int> &clause, int dd_index = 0);
    // adds an exact one constraint to the root node
    void add_exactly_one_primitive(std::vector<int> &variables, int dd_index = 0);
    // adds a dnf to the bdd. it has the form 'g or (a^b^c) or (x^y)'
    void add_dnf_primitive(std::vector<std::vector<int>> &dnf, int dd_index = 0);

    void create_ith_var(int i);

    // Functions for building the bdd timestep by timestep
    // permutes the variables in source bdd according to the given permutation
    // write the result into destionation bdd. the old bdd in destination gets freed and is lost
    void permute_variables(std::vector<int> &permutation, int source_bdd, int destination_bdd);
    // conjoins bdd a and b and stores the result. the old destination bdd get freed and is lost
    void conjoin_two_dds(int dd_a, int dd_b, int dd_result);
    //  returns a node for the main bdd_manager that represents the bdd for a single timestep
    void copy_and_conjoin_bdd_from_another_container(bdd_container &copy_from);

    // stops automatic reordering and reactivates it
    void disable_reordering();
    void enable_reordering();
    // groups a set of variables.
    // the group has to be consecutive in the current variable order.
    // the lower range end of the group starts at the variable with index low (not position low)
    // from this index on the next size variables are added to the group
    // they will stay consecutive in the variable order from now on
    void set_variable_group(int low, int size);
    // The i-th entry of the permutation array contains the index of the variable that should be brought to the i-th
    // layer -> var index
    void set_variable_order(std::vector<int> &variable_order);
    // returns the permutation of the variable order
    // the i-th entry contains the variable index that currently resides in the i-th layer
    // layer -> var index
    std::vector<int> get_variable_order();
    // reorders the variables with the reorder_shift_converge method
    void reduce_heap();

    // Wirtes all the information about the CUDD manager to std::out
    void print_bdd_info();
    // lists all minterms of the bdd (satisficing assignments). Lists up to max of such minterms.
    std::vector<std::vector<bool>> list_minterms(int max);
    // returns a short string with a few information about the current stae of CUDD
    // can be used to evaluate the progress of CUDD during the execution of the program
    virtual std::string get_short_statistics(int bdd_index = 0);
    // writes the bdd to a file in dot format
    void write_bdd_to_dot_file(std::string filename);

    // function is purely for debugging purpose. allows entry point to bdd manager
    void hack_back_rocket_method();

    // returns the variable order for timesetep 0 as used by the sub manager
    // maps var idx -> layer in bdd (there are no gaps in the layers)
    std::map<int, int> get_variable_order_for_single_step(std::map<planning_logic::tagged_variable, int> &variable_map);
    // extends the variable order for timestep 0 to all timesteps
    std::vector<int> extend_variable_order_to_all_steps(std::map<planning_logic::tagged_variable, int> &variable_map,
                                                        std::map<int, int> &single_step_order);
};