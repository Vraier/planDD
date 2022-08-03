#pragma once

#include "logic_primitive.h"

namespace encoder {
class encoder {
   public:
    virtual ~encoder(){};

    // clears all old dds and initialalizes num new dds
    virtual std::vector<planning_logic::logic_primitive> get_logic_primitives(planning_logic::primitive_tag tag,
                                                                              int timestep) = 0;
};
}  // namespace encoder