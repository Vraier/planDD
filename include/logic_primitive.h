#pragma once
#include <vector>

namespace planning_logic {

enum logic_primitive_type {
    logic_clause,
    logic_eo,
};

enum primitive_tag {
    ini_state,
    goal,
    al_var,  // at least one var is true
    am_var,  // at most one var is true
    al_op,
    am_op,
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

    logic_primitive(logic_primitive_type type, primitive_tag c_tag, int timesteps, std::vector<int> data)
        : m_type(type), m_clause_tag(c_tag), m_timestep(timesteps), m_data(data) {}
};
}  // namespace planning_logic