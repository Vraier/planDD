#include "sdd_container.h"

#include "logging.h"

// TODO implement all these things
sdd_container::sdd_container(int num_sdds, int var_count) {
    int auto_gc_and_minimize = 1;  // will automatically do reordering if not zero
    m_sdd_manager = sdd_manager_create(var_count, auto_gc_and_minimize);

    m_root_nodes = std::vector<SddNode *>(num_sdds);
    for (int i = 0; i < num_sdds; i++) {
        m_root_nodes[i] = sdd_manager_true(m_sdd_manager);
        sdd_ref(m_root_nodes[i], m_sdd_manager);
    }
}

sdd_container::~sdd_container() {
    for (int i = 0; i < m_root_nodes.size(); i++) {
        sdd_deref(m_root_nodes[i], m_sdd_manager);
    }
    sdd_manager_free(m_sdd_manager);
}

void sdd_container::set_num_dds(int num_dds) {
    for (int i = 0; i < m_root_nodes.size(); i++) {
        sdd_deref(m_root_nodes[i], m_sdd_manager);
    }

    // create new dds
    m_root_nodes = std::vector<SddNode *>(num_dds);
    for (int i = 0; i < num_dds; i++) {
        // force var with index 0 to be true
        m_root_nodes[i] = sdd_manager_literal(0, m_sdd_manager);
        sdd_ref(m_root_nodes[i], m_sdd_manager);
    }
}

bool sdd_container::is_constant_false(int dd_index) { return sdd_node_is_true(m_root_nodes[dd_index]) == 1; }
double sdd_container::count_num_solutions(int dd_index) { return sdd_model_count(m_root_nodes[0], m_sdd_manager); }

void sdd_container::clear_dd(int dd_index) {
    sdd_deref(m_root_nodes[dd_index], m_sdd_manager);
    m_root_nodes[dd_index] = sdd_manager_literal(0, m_sdd_manager);
    sdd_ref(m_root_nodes[dd_index], m_sdd_manager);
}

void sdd_container::add_clause_primitive(std::vector<int> &clause, int sdd_index) {
    // build the disjunction of the literals in the clause

    SddNode *var, *tmp;
    SddNode *disjunction = sdd_manager_false(m_sdd_manager);
    sdd_ref(disjunction, m_sdd_manager);

    for (int i = 0; i < clause.size(); i++) {
        int literal = clause[i];
        // we do not have to take the abs value here. sdd library can handle negative values
        var = sdd_manager_literal(literal, m_sdd_manager);

        tmp = sdd_disjoin(disjunction, var, m_sdd_manager);

        sdd_ref(tmp, m_sdd_manager);
        sdd_deref(disjunction, m_sdd_manager);
        disjunction = tmp;
    }

    // conjoin the clause with the root node
    tmp = sdd_conjoin(m_root_nodes[sdd_index], disjunction, m_sdd_manager);
    sdd_ref(tmp, m_sdd_manager);
    sdd_deref(m_root_nodes[sdd_index], m_sdd_manager);
    sdd_deref(disjunction, m_sdd_manager);
    m_root_nodes[sdd_index] = tmp;
}

void sdd_container::add_exactly_one_primitive(std::vector<int> &variables, int sdd_index) {
    LOG_MESSAGE(log_level::error) << "Exactly one constraint not supported for SDDs.";
    return;
}
void sdd_container::add_dnf_primitive(std::vector<std::vector<int>> &dnf,
                                      int sdd_index) {  // build the disjunction of the literals in the clause
    SddNode *var, *tmp;
    SddNode *disjunction = sdd_manager_false(m_sdd_manager);
    sdd_ref(disjunction, m_sdd_manager);

    for (int i = 0; i < dnf.size(); i++) {
        SddNode *conjunction = sdd_manager_true(m_sdd_manager);
        sdd_ref(conjunction, m_sdd_manager);

        for (int j = 0; j < dnf[i].size(); j++) {
            var = sdd_manager_literal(dnf[i][j], m_sdd_manager);  // can handle negative literals
            tmp = sdd_conjoin(var, conjunction, m_sdd_manager);

            sdd_ref(tmp, m_sdd_manager);
            sdd_deref(conjunction, m_sdd_manager);
            conjunction = tmp;
        }

        tmp = sdd_disjoin(disjunction, conjunction, m_sdd_manager);
        sdd_ref(tmp, m_sdd_manager);
        sdd_deref(disjunction, m_sdd_manager);
        sdd_deref(conjunction, m_sdd_manager);
        disjunction = tmp;
    }

    // conjoin the clause with the root node
    tmp = sdd_conjoin(m_root_nodes[sdd_index], disjunction, m_sdd_manager);
    sdd_ref(tmp, m_sdd_manager);
    sdd_deref(m_root_nodes[sdd_index], m_sdd_manager);
    sdd_deref(disjunction, m_sdd_manager);
    m_root_nodes[sdd_index] = tmp;
}

void sdd_container::create_ith_var(int i) { sdd_manager_literal(i, m_sdd_manager); }

void sdd_container::permute_variables(std::vector<int> &permutation, int source_bdd, int destination_bdd) {
    LOG_MESSAGE(log_level::error) << "Permuting variables not supported for SDDs.";
    return;
}
void sdd_container::conjoin_two_dds(int dd_a, int dd_b, int dd_result) {
    LOG_MESSAGE(log_level::info) << "Conjoining two sdds";
    SddNode *tmp = sdd_conjoin(m_root_nodes[dd_a], m_root_nodes[dd_b], m_sdd_manager);
    sdd_ref(tmp, m_sdd_manager);
    sdd_deref(m_root_nodes[dd_result], m_sdd_manager);
    m_root_nodes[dd_result] = tmp;
}

void sdd_container::disable_reordering() { sdd_manager_auto_gc_and_minimize_on(m_sdd_manager); }
void sdd_container::enable_reordering() { sdd_manager_auto_gc_and_minimize_off(m_sdd_manager); }
void sdd_container::reduce_heap() { sdd_manager_minimize(m_sdd_manager); }

void sdd_container::print_info() {
    LOG_MESSAGE(log_level::info) << "Printing LibSDD statistics...";
    sdd_manager_print(m_sdd_manager);
    SddModelCount count = sdd_model_count(m_root_nodes[0], m_sdd_manager);
    LOG_MESSAGE(log_level::info) << "Counted a total of " << count << " models";
}

std::string sdd_container::get_short_statistics(int sdd_index) { return ""; }