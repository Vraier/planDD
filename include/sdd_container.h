#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "dd_buildable.h"

extern "C" {
#include "sddapi.h"
#include "compiler.h" // compiler has to be included after sdd api... cool
}

class sdd_manager : public virtual dd_buildable  {
   private:
    /* data */
    SddManager *m_sdd_manager;
    SddNode *m_root_node;

   public:
    sdd_manager(int num_variables);
    ~sdd_manager();

    void print_sdd();
    virtual std::string get_short_statistics(int sdd_index = 0);

    virtual void conjoin_clause(std::vector<int> &clause, int sdd_index = 0);
    virtual void add_exactly_one_constraint(std::vector<int> &variables, int sdd_index = 0);
};

/*
SddCompilerOptions sdd_options = {
    NULL,       NULL, NULL, NULL, NULL, NULL, NULL, NULL,  // file names
    0,                                                     // flag
    "balanced",                                            // vtree type
    -1,                                                    // vtree search mode
    0,                                                     // post-compilation search
    1                                                      // verbose
};

SddManager *m = sdd_manager_create(encoder.num_cnf_variables(), 0);
sdd_manager_set_options(&sdd_options, m);

LOG_MESSAGE(log_level::info) << "Created SDD manager";
Cnf *sdd_cnf = read_cnf(options.m_values.cnf_file.c_str());
printf("vars=%ld clauses=%zu\n", sdd_cnf->var_count, sdd_cnf->litset_count);
LOG_MESSAGE(log_level::info) << "Read CNF";
SddNode *sdd = fnf_to_sdd(sdd_cnf, m);
LOG_MESSAGE(log_level::info) << "Finished building SDD";

sdd_manager_print(m);
*/
