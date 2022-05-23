#pragma once

#include <vector>

class dd_buildable
{
public:
    virtual ~dd_buildable() {};

    virtual void conjoin_clause(std::vector<int> &clause) = 0;
    virtual std::string get_short_statistics() = 0;
};