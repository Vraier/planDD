#ifndef H_DD_BUILDER
#define H_DD_BUILDER

#include <map>
#include <vector>

#include "dd_buildable.h"
#include "cnf.h"

namespace dd_builder
{    
    void construct_dd_linear_disjoint(dd_buildable &dd, planning_cnf::cnf &cnf, std::string disjoint_order, bool reversed);
}

#endif