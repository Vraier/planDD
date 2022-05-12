#ifndef H_BDD_MANAGER
#define H_BDD_MANAGER

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

   public:
    bdd_manager();
    virtual ~bdd_manager();

    // reorders the variables
    void reduce_heap();

    // returns the permutation of the variable order
    // the i-th entry contains the the level in the BDD in which the variable resides in
    std::vector<int> get_variable_order(int num_variables);


    void print_bdd(int num_variables);
    virtual std::string get_short_statistics();
    void write_bdd_to_dot_file(std::string filename);

    virtual void conjoin_clause(std::vector<int> &clause);
};

#endif
