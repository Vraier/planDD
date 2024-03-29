#include "logic_primitive.h"

#include <set>

#include "logging.h"

namespace planning_logic {

std::vector<int> logic_primitive::get_affected_variables() const {
    switch (m_type) {
        case logic_clause:
        case logic_eo: {
            std::set<int> affected_vars;

            for (int i = 0; i < m_data.size(); i++) {
                affected_vars.insert(std::abs(m_data[i]));
            }
            return std::vector<int>(affected_vars.begin(), affected_vars.end());
        }
        case logic_dnf: {
            std::set<int> affected_vars;

            for (int i = 0; i < m_dnf_data.size(); i++) {
                for (int j = 0; j < m_dnf_data[i].size(); j++) {
                    affected_vars.insert(std::abs(m_dnf_data[i][j]));
                }
            }
            return std::vector<int>(affected_vars.begin(), affected_vars.end());
        }
        default:
            LOG_MESSAGE(log_level::error) << "Unknown primitive type";
            return std::vector<int>();
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

bool logic_primitive::operator<(const logic_primitive& rhs) const {
    if (m_type == rhs.m_type) {
        if (m_data.size() != 0) {
            return m_data < rhs.m_data;
        } else {
            return m_dnf_data < rhs.m_dnf_data;
        }
    } else {
        return m_type < rhs.m_type;
    }
}
}  // namespace planning_logic