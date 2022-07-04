#pragma once
#include <vector>

enum logic_primitive_type {
    logic_clause,
    logic_eo,
};

enum clause_tag {
    clause_ini_state,
    clause_goal,
    clause_al_var,  // at least one var is true
    clause_am_var,  // at most one var is true
    clause_al_op,
    clause_am_op,
    clause_mutex,
    clause_precon,
    clause_effect,
    clause_frame,
    clause_none,
};

// exactly one variable is true
enum eo_constraint_tag {
    eo_var,
    eo_op,
    eo_none,
};

class logic_primitive {
   private:
   public:
    logic_primitive_type m_type;
    clause_tag m_clause_tag;
    eo_constraint_tag m_eo_constraint_tag;

    int m_timestep;
    std::vector<int> m_data;

    logic_primitive(logic_primitive_type type, clause_tag c_tag, eo_constraint_tag eo_tag, int timesteps,
                    std::vector<int> data)
        : m_type(type), m_clause_tag(c_tag), m_eo_constraint_tag(eo_tag), m_timestep(timesteps), m_data(data) {}
};
