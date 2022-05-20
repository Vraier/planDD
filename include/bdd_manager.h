#ifndef H_BDD_MANAGER
#define H_BDD_MANAGER

#include <iostream>
#include <string>
#include <vector>

#include "dd_buildable.h"

extern "C" {
#include "cudd.h"
#include "util.h"
#include "cuddInt.h"
}

// code wont compile if i #include <iostream> after util.h

class bdd_manager : public virtual dd_buildable {
   private:
    /* data */
    DdManager *m_bdd_manager;
    DdNode *m_root_node;
    int m_num_variables;
    // a permutation that tells the bdd manager at which layer variable i should lay in.
    std::vector<int> m_initial_variable_order;
    std::vector<int> m_inverse_initial_variable_order;

   public:
    bdd_manager(int num_variables);
    bdd_manager(int num_variables, std::vector<int> &inital_variable_order);
    virtual ~bdd_manager();

    // reorders the variables
    void reduce_heap();

    // returns the permutation of the variable order
    // the i-th entry contains the the level in the BDD in which the variable resides in
    std::vector<int> get_variable_order(int num_variables);

    // Wirtes all the information about the CUDD manager to std::out
    void print_bdd();
    // returns a short string with a few information about the current stae of CUDD
    // can be used to evaluate the progress of CUDD during the execution of the program
    virtual std::string get_short_statistics();
    // writes the bdd to a file in dot format
    void write_bdd_to_dot_file(std::string filename);

    virtual void conjoin_clause(std::vector<int> &clause);

    // builds a bdd that is true iff exactly one of the variables is true
    // conjoins the bdd with the root node
    // should contain at least one variable
    void add_exactly_one_constraint(std::vector<int> &variables);
    // function is purely for debugging purpose. allows entry point to bdd manager
    void hack_back_rocket_method();
};

#endif
