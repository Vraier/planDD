#include "logic_primitive.h"

#include <set>

#include "logging.h"

namespace planning_logic {

std::vector<int> logic_primitive::get_affected_variables() {
    switch (m_type) {
        case logic_clause:
            return m_data;
        case logic_dnf: {
            std::set<int> affected_vars;
            for (int i = 0; i < m_dnf_data.size(); i++) {
                for (int j = 0; j < m_dnf_data[i].size(); j++) {
                    affected_vars.insert(m_dnf_data[i][j]);
                }
            }
            return std::vector<int>(affected_vars.begin(), affected_vars.end());
        }
        case logic_eo:
            return m_data;
        default:
            LOG_MESSAGE(log_level::error) << "Unknown primitive type";
            std::vector<int>();
    }
}

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
            LOG_MESSAGE(log_level::error) << "Unknown primitive type";
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