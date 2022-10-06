#include "sdd_container.h"

#include "logging.h"

// TODO implement all these things
sdd_manager::sdd_manager(int num_variables) {
    int auto_gc_and_minimize = 1;  // will automatically do reordering if not zero
    m_sdd_manager = sdd_manager_create(num_variables, auto_gc_and_minimize);

    m_root_node = sdd_manager_true(m_sdd_manager);
    sdd_ref(m_root_node, m_sdd_manager);
}

sdd_manager::~sdd_manager() { sdd_manager_free(m_sdd_manager); }

void sdd_manager::set_num_dds(int num_dds) { return; }

bool sdd_manager::is_constant_false(int dd_index) { return true; }
double sdd_manager::count_num_solutions(int dd_index){ return 0.0;}

void sdd_manager::clear_dd(int dd_index) { return; }

void sdd_manager::add_clause_primitive(std::vector<int> &clause, int sdd_index) {
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
    tmp = sdd_conjoin(m_root_node, disjunction, m_sdd_manager);

    sdd_ref(tmp, m_sdd_manager);
    sdd_deref(m_root_node, m_sdd_manager);
    sdd_deref(disjunction, m_sdd_manager);
    m_root_node = tmp;
}

void sdd_manager::add_exactly_one_primitive(std::vector<int> &variables, int sdd_index) { return; }
void sdd_manager::add_dnf_primitive(std::vector<std::vector<int>> &dnf, int sdd_index) { return; }

void sdd_manager::create_ith_var(int i) { return; }

void sdd_manager::permute_variables(std::vector<int> &permutation, int source_bdd, int destination_bdd) { return; }
void sdd_manager::conjoin_two_dds(int dd_a, int dd_b, int dd_result) { return; }

void sdd_manager::disable_reordering() { return; }
void sdd_manager::enable_reordering() { return; }
void sdd_manager::reduce_heap() { return; }

void sdd_manager::print_sdd() {
    LOG_MESSAGE(log_level::info) << "Printing LibSDD statistics...";
    SddModelCount count = sdd_model_count(m_root_node, m_sdd_manager);
    LOG_MESSAGE(log_level::info) << "Counted a total of " << count << " models";

    sdd_manager_print(m_sdd_manager);
}

std::string sdd_manager::get_short_statistics(int sdd_index) { return ""; }