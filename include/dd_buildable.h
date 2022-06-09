#pragma once

#include <vector>
#include <string>

class dd_buildable
{
public:
    virtual ~dd_buildable() {};

    virtual void conjoin_clause(std::vector<int> &clause, int dd_index = 0) = 0;
    // builds a bdd that is true iff exactly one of the variables is true
    // conjoins the bdd with the root node
    // should contain at least one variable
    virtual void add_exactly_one_constraint(std::vector<int> &variables, int dd_index = 0) = 0;
    virtual std::string get_short_statistics(int dd_index = 0) = 0;
};