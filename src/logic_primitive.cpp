#include "logic_primitive.h"

namespace planning_logic {

std::string logic_primitive::to_string() {
    std::string type, time, data;
    type = m_type == logic_clause ? "cl" : "eo";
    time = "t=" + std::to_string(m_timestep);
    data = "d=";
    for(int x: m_data){
        data += std::to_string(x) + " ";
    }
    return type + " " + time + " " + data;
}
}  // namespace planning_logic