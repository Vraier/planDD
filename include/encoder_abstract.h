#pragma once

#include "logic_primitive.h"
#include "plan_to_cnf_map.h"
#include "sas_parser.h"
#include "options.h"

namespace encoder {
class encoder_abstract {
   protected:
    // holds options for the whole programm. Some are important for the cnf_encoder
    option_values m_options;

    encoder_abstract(option_values &options, sas_problem &problem, int num_operators)
        : m_options(options), m_sas_problem(problem), m_symbol_map(num_operators) {}

   public:
    // represents the planning problem
    sas_problem m_sas_problem;
    // maps planning variables to cnf variables.
    planning_logic::plan_to_cnf_map m_symbol_map;

    virtual ~encoder_abstract(){};
    virtual std::vector<planning_logic::logic_primitive> get_logic_primitives(planning_logic::primitive_tag tag,
                                                                              int timestep) = 0;
};
}  // namespace encoder