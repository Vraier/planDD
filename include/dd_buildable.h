#ifndef H_DD_BUILDABLE
#define H_DD_BUILDABLE

#include <vector>

class dd_buildable
{
public:
    virtual ~dd_buildable() {};

    virtual void conjoin_clause(std::vector<int> &clause) = 0;
};

#endif
