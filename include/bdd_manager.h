#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "dd_buildable.h"

extern "C" {
#include "cudd.h"
#include "util.h"
}

// code wont compile if i #include <iostream> after util.h

class bdd_manager : public virtual dd_buildable {
   private:
    /* data */
    DdManager *m_bdd_manager;
    DdNode *m_root_node;
    int m_num_variables;

   public:
    bdd_manager(int num_variables);
    virtual ~bdd_manager();

    // reorders the variables
    void reduce_heap();

    // The i-th entry of the permutation array contains the index of the variable that should be brought to the i-th level
    void set_variable_order(std::vector<int> &variable_order);
    // returns the permutation of the variable order
    // the i-th entry contains the the level in the BDD in which the variable resides in
    std::vector<int> get_variable_order();

    // Wirtes all the information about the CUDD manager to std::out
    void print_bdd();
    // returns a short string with a few information about the current stae of CUDD
    // can be used to evaluate the progress of CUDD during the execution of the program
    virtual std::string get_short_statistics();
    // writes the bdd to a file in dot format
    void write_bdd_to_dot_file(std::string filename);

    virtual void conjoin_clause(std::vector<int> &clause);
    virtual void add_exactly_one_constraint(std::vector<int> &variables);
    
    // function is purely for debugging purpose. allows entry point to bdd manager
    void hack_back_rocket_method();
};