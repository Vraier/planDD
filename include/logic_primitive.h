#pragma once
#include <vector>
#include <string>

namespace planning_logic {

enum logic_primitive_type {
    logic_clause,
    logic_dnf,
    logic_eo,
};

enum primitive_tag {
    ini_state,
    goal,
    eo_var,  // exact one var is true
    eo_op,
    mutex,
    precon,
    effect,
    frame,
    none,
};

class logic_primitive {
   private:
   public:
    logic_primitive_type m_type;
    primitive_tag m_clause_tag;

    int m_timestep;
    std::vector<int> m_data;
    std::vector<std::vector<int>> m_dnf_data;

    logic_primitive(logic_primitive_type type, primitive_tag c_tag, int timesteps, std::vector<int> data)
        : m_type(type), m_clause_tag(c_tag), m_timestep(timesteps), m_data(data) {}
    logic_primitive(logic_primitive_type type, primitive_tag c_tag, int timesteps,
                    std::vector<std::vector<int>> dnf_data)
        : m_type(type), m_clause_tag(c_tag), m_timestep(timesteps), m_dnf_data(dnf_data) {}

    std::vector<int> get_affected_variables() const;
    std::string to_string();

    bool operator <(const logic_primitive& rhs) const;
};
}  // namespace planning_logic