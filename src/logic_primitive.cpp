#include "logic_primitive.h"

namespace planning_logic {

std::string logic_primitive::to_string() {
    std::string type, time, data;
    switch (m_type) {
        case logic_clause:
            type = "cl";
            break;
        case logic_dnf:
            type = "dnf";
            break;
        case logic_eo:
            type = "eo";
            break;
        default:
            type = "none";
            break;
    }
    type = m_type == logic_clause ? "cl" : "eo";
    time = "t=" + std::to_string(m_timestep);
    data = "d=";
    for (int x : m_data) {
        data += std::to_string(x) + " ";
    }
    for (std::vector<int> con : m_dnf_data) {
        for (int x : con) {
            data += std::to_string(x) + " ";
        }
        data += ",";
    }
    return type + " " + time + " " + data;
}
}  // namespace planning_logic