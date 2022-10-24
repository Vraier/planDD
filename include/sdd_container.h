#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "dd_buildable.h"

extern "C" {
#include "sddapi.h"
#include "compiler.h"  // compiler has to be included after sdd api... cool
}

class sdd_container : public virtual dd_buildable {
   private:
    /* data */
    SddManager *m_sdd_manager;
    std::vector<SddNode *> m_root_nodes;

   public:
    sdd_container(int num_sdds, int var_count);
    ~sdd_container();
    void set_num_dds(int num_dds);

    bool is_constant_false(int dd_index);
    double count_num_solutions(int dd_index);

    void clear_dd(int dd_index = 0);

    void add_clause_primitive(std::vector<int> &clause, int sdd_index = 0);
    void add_exactly_one_primitive(std::vector<int> &variables, int sdd_index = 0);
    void add_dnf_primitive(std::vector<std::vector<int>> &dnf, int bdd_index = 0);
    
    void create_ith_var(int i);
    
    void permute_variables(std::vector<int> &permutation, int source_bdd, int destination_bdd);
    void conjoin_two_dds(int dd_a, int dd_b, int dd_result);

    void disable_reordering();
    void enable_reordering();
    void reduce_heap();

    void print_info();
    std::string get_short_statistics(int dd_index = 0);
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
