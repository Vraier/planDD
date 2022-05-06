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

    // returns the permutation of the variable order
    // the i-th entry contains the the level in the BDD in which the variable resides in
    std::vector<int> get_variable_order(int num_variables);


    void print_bdd(int num_variables);
    void write_bdd_to_dot_file(DdNode *bdd, std::string filename);
    //DdNode *construct_bdd_from_cnf(std::vector<std::vector<int>> &cnf);
    //DdNode *construct_bdd_from_cnf_binary(std::vector<std::vector<int>> &cnf);

    virtual void conjoin_clause(std::vector<int> &clause);
};

#endif
