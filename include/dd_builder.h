#ifndef H_DD_BUILDER
#define H_DD_BUILDER

#include <map>
#include <vector>

#include "dd_buildable.h"
#include "cnf.h"

namespace dd_builder
{
    typedef std::vector<int> clause;

    std::map<planning_cnf::clause_tag, std::vector<std::vector<clause>>> categorize_clauses(planning_cnf::cnf &cnf);
    
    void construct_dd_linear_disjoint(dd_buildable &dd, planning_cnf::cnf &cnf, std::string disjoint_order, bool reversed);
    void construct_dd_linear_interleaved(dd_buildable &dd, planning_cnf::cnf &cnf, std::string interleaved_order, bool reversed);
}

#endif