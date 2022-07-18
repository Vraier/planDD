#pragma once

#include "options.h"

namespace planDD {
int hack_debug(option_values opt_values);

int build_bdd(option_values opt_values);
int build_sdd(option_values opt_values);

int encode_cnf(option_values opt_values);
int cnf_to_bdd(option_values opt_values);

int single_minisat(option_values opt_values);
int count_minisat(option_values opt_values);
};