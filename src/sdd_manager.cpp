#include "sdd_manager.h"

#include "logging.h"

sdd_manager::sdd_manager(int num_variables) {
    int auto_gc_and_minimize = 1; // will automatically do reordering if not zero
    m_sdd_manager = sdd_manager_create(num_variables, auto_gc_and_minimize);

    m_root_node = sdd_manager_true(m_sdd_manager);
    sdd_ref(m_root_node, m_sdd_manager);
}

sdd_manager::~sdd_manager() { 
    sdd_manager_free(m_sdd_manager); 
}

void sdd_manager::conjoin_clause(std::vector<int> &clause) {
    // build the disjunction of the literals in the clause
    
    SddNode *var, *tmp;
    SddNode *disjunction = sdd_manager_false(m_sdd_manager);
    sdd_ref(disjunction, m_sdd_manager);

    for(int i = 0; i < clause.size(); i++){
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

    /*
    DdNode *var, *tmp;
    DdNode *disjunction = Cudd_ReadLogicZero(m_bdd_manager);
    Cudd_Ref(disjunction);

    for (int j = 0; j < clause.size(); j++) {
        int literal = clause[j];
        var = Cudd_bddIthVar(m_bdd_manager, std::abs(literal));

        if (literal > 0) {
            tmp = Cudd_bddOr(m_bdd_manager, var, disjunction);
        } else {
            tmp = Cudd_bddOr(m_bdd_manager, Cudd_Not(var), disjunction);
        }
        Cudd_Ref(tmp);
        Cudd_RecursiveDeref(m_bdd_manager, disjunction);
        disjunction = tmp;
    }

    // conjoin the clause with the root node
    tmp = Cudd_bddAnd(m_bdd_manager, m_root_node, disjunction);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    Cudd_RecursiveDeref(m_bdd_manager, disjunction);
    m_root_node = tmp;
    */
}

void sdd_manager::print_sdd(){
    SddModelCount count = sdd_model_count(m_root_node, m_sdd_manager);
    LOG_MESSAGE(log_level::info) << "Counted a total of " << count << " models";
}