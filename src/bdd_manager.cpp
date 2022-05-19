#include "logging.h"

#include "bdd_manager.h"

bdd_manager::bdd_manager(int num_variables) {
    m_bdd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    Cudd_AutodynEnable(m_bdd_manager, CUDD_REORDER_SIFT);
    //Cudd_SetMaxCacheHard(m_bdd_manager, 62500000);

    m_root_node = Cudd_ReadOne(m_bdd_manager);
    Cudd_Ref(m_root_node);

    // build identity for inital variable order
    m_num_variables = num_variables;
    m_initial_variable_order = std::vector<int>(num_variables+1);
    for(int i = 0; i <= m_num_variables; i++){
        m_initial_variable_order[i] = i;
    }
    m_inverse_initial_variable_order = m_initial_variable_order;

    // TODO: does this work?
    // tell CUDD how many variables to expect
    Cudd_bddIthVar(m_bdd_manager, m_num_variables);
}

bdd_manager::bdd_manager(int num_variables, std::vector<int> &initial_variable_order) {
    m_bdd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    Cudd_AutodynEnable(m_bdd_manager, CUDD_REORDER_SIFT);
    //Cudd_SetMaxCacheHard(m_bdd_manager, 62500000);

    m_root_node = Cudd_ReadOne(m_bdd_manager);
    Cudd_Ref(m_root_node);

    m_num_variables = num_variables;
    m_initial_variable_order = initial_variable_order;

    // invert the inital variable order
    m_inverse_initial_variable_order = std::vector<int>(m_initial_variable_order.size());
    for(int i = 0; i < m_initial_variable_order.size(); i++){
        m_inverse_initial_variable_order[m_initial_variable_order[i]] = i;
    }

    // TODO: does this work?
    // tell CUDD how many variables to expect
    Cudd_bddIthVar(m_bdd_manager, m_num_variables);   
}

bdd_manager::~bdd_manager() { 
    Cudd_Quit(m_bdd_manager); 
}

void bdd_manager::reduce_heap(){
    LOG_MESSAGE(log_level::info) << "Start reducing heap manually";
    Cudd_ReduceHeap(m_bdd_manager, CUDD_REORDER_SIFT_CONVERGE, 1);
    LOG_MESSAGE(log_level::info) << "Finished reducing heap";
}

void bdd_manager::print_bdd() {
    LOG_MESSAGE(log_level::info) << "Printing CUDD statistics...";
    LOG_MESSAGE(log_level::info) << "Number of nodes: " << Cudd_DagSize(m_root_node) << ", Number of solutions: "
                                 << Cudd_CountMinterm(m_bdd_manager, m_root_node, m_num_variables);
    FILE** fout = &stdout;
    Cudd_PrintInfo(m_bdd_manager, *fout);
}

std::string bdd_manager::get_short_statistics(){
    long num_nodes = Cudd_ReadNodeCount(m_bdd_manager);
    long num_peak_nodes = Cudd_ReadPeakNodeCount(m_bdd_manager);
    long num_reorderings = Cudd_ReadReorderings(m_bdd_manager);
    long num_mem_bytes = Cudd_ReadMemoryInUse(m_bdd_manager);
    std::string result = "CUDD stats: #nodes: " + std::to_string(num_nodes)
                                    + " #peak nodes: " + std::to_string(num_peak_nodes)
                                    + " #reorderings: " + std::to_string(num_reorderings)
                                    + " #memory bytes: " + std::to_string(num_mem_bytes);

    return result;
}

// Writes a dot file representing the argument DDs
void bdd_manager::write_bdd_to_dot_file(std::string filename) {
    DdNode *add = Cudd_BddToAdd(m_bdd_manager, m_root_node);
    Cudd_Ref(add);
    FILE *outfile;  // output file pointer for .dot file
    outfile = fopen(filename.c_str(), "w");
    DdNode **ddnodearray = (DdNode **)malloc(sizeof(DdNode *));  // initialize the function array
    ddnodearray[0] = add;
    Cudd_DumpDot(m_bdd_manager, 1, ddnodearray, NULL, NULL, outfile);  // dump the function to .dot file
    free(ddnodearray);
    Cudd_RecursiveDeref(m_bdd_manager, add);
    fclose(outfile);
}

void bdd_manager::conjoin_clause(std::vector<int> &clause) {
    // build the disjunction of the literals in the clause
    DdNode *var, *tmp;
    DdNode *disjunction = Cudd_ReadLogicZero(m_bdd_manager);
    Cudd_Ref(disjunction);

    for (int j = 0; j < clause.size(); j++) {
        int unpermuted_literal = clause[j];
        var = Cudd_bddIthVar(m_bdd_manager, m_initial_variable_order[std::abs(unpermuted_literal)]);

        if (unpermuted_literal > 0) {
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
}

void bdd_manager::add_exactly_one_constraint(std::vector<int> &variables){

    // order variables by the variable order
    // variable in the lowes layer comes first
    // variable in the highest layer comes last
    std::vector<int> ordered_variables; // TODO really order them by variable order
    for(int i = variables.size()-1; i >= 0; i--){
        ordered_variables.push_back(variables[i]);
    }

    DdNode *false_node = Cudd_ReadLogicZero(m_bdd_manager); Cudd_Ref(false_node);
    DdNode *exact_one_true = Cudd_ReadOne(m_bdd_manager); Cudd_Ref(exact_one_true);
    DdNode *exact_zero_true = Cudd_ReadLogicZero(m_bdd_manager); Cudd_Ref(exact_zero_true);

    // from bottom to top ignoring the topmost variable
    for(int i = 0; i < ordered_variables.size()-1; i++){

        // create nodes for new layer
        // cuddUniqueInter searches the unique table for the given node and creates a new one if it does not exist
        // have to be careful about the variable order when using cuddUniqueInter
        DdNode *new_exact_one_true = cuddUniqueInter(m_bdd_manager, ordered_variables[i], false_node, exact_one_true);
        Cudd_Ref(new_exact_one_true);
        DdNode *new_exact_zero_true = cuddUniqueInter(m_bdd_manager, ordered_variables[i], exact_one_true, exact_zero_true);
        Cudd_Ref(new_exact_zero_true);

        // clean up old nodes
        //Cudd_RecursiveDeref(m_bdd_manager, exact_one_true);
        //Cudd_RecursiveDeref(m_bdd_manager, exact_zero_true);
        exact_one_true = new_exact_one_true;
        exact_zero_true = new_exact_zero_true;
    }

    // build the root node for the last variable
    DdNode *constraint_root_node = cuddUniqueInter(m_bdd_manager, ordered_variables[ordered_variables.size()-1], exact_one_true, exact_zero_true);
    Cudd_Ref(constraint_root_node);
    //Cudd_RecursiveDeref(m_bdd_manager, exact_one_true);
    //Cudd_RecursiveDeref(m_bdd_manager, exact_zero_true);

    // conjoin the root node for the exact one constraint with the root node of the bdd
    DdNode *tmp = Cudd_bddAnd(m_bdd_manager, m_root_node, constraint_root_node);
    Cudd_Ref(tmp);
    //Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    //Cudd_RecursiveDeref(m_bdd_manager, constraint_root_node);
    m_root_node = tmp;
}

// This method returns the variable permutation
// the ith entry of the return vector dictates what variable (index) resides in the ith layer of the bdd
std::vector<int> bdd_manager::get_variable_order(int num_variables){
    std::vector<int> permutation(num_variables+1, -1);
    for(int i = 0; i <= num_variables; i++){
        int index_in_bdd_word = m_initial_variable_order[i];
        // tells us at what layer the var resides in
        int perm_pos = Cudd_ReadPerm(m_bdd_manager, index_in_bdd_word);
        if (perm_pos == -1) { // variable does not exist
            continue;
        }
        permutation[perm_pos] = i;
    }   
    return permutation;
}

// TODO: find out what swapvars, bddpermute does
// what is a variable map?