#include "logging.h"

#include "bdd_manager.h"

bdd_manager::bdd_manager(int num_variables) {
    // add one more variable to account for variable with index 0
    m_num_variables = num_variables + 1;

    m_bdd_manager = Cudd_Init(m_num_variables, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    Cudd_AutodynEnable(m_bdd_manager, CUDD_REORDER_SIFT);
    // Cudd_SetMaxCacheHard(m_bdd_manager, 62500000);

    // force var with index 0 to be true
    m_root_node = Cudd_bddIthVar(m_bdd_manager, 0);
    Cudd_Ref(m_root_node);
}

bdd_manager::~bdd_manager() {
    Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    int num_zero_reference_nodes = Cudd_CheckZeroRef(m_bdd_manager);
    if (num_zero_reference_nodes != 0) {
        LOG_MESSAGE(log_level::warning) << "#Nodes with non-zero reference count (should be 0): "
                                        << Cudd_CheckZeroRef(m_bdd_manager);
    }

    Cudd_Quit(m_bdd_manager);
}

void bdd_manager::reduce_heap() {
    LOG_MESSAGE(log_level::info) << "Start reducing heap manually";
    Cudd_ReduceHeap(m_bdd_manager, CUDD_REORDER_SIFT_CONVERGE, 1);
    LOG_MESSAGE(log_level::info) << "Finished reducing heap";
}

void bdd_manager::print_bdd() {
    LOG_MESSAGE(log_level::info) << "Printing CUDD statistics...";
    LOG_MESSAGE(log_level::info) << "Number of nodes: " << Cudd_DagSize(m_root_node) << ", Number of solutions: "
                                 << Cudd_CountMinterm(m_bdd_manager, m_root_node, m_num_variables);
    FILE **fout = &stdout;
    Cudd_PrintInfo(m_bdd_manager, *fout);
}

std::string bdd_manager::get_short_statistics() {
    long num_nodes = Cudd_ReadNodeCount(m_bdd_manager);
    long num_peak_nodes = Cudd_ReadPeakNodeCount(m_bdd_manager);
    long num_reorderings = Cudd_ReadReorderings(m_bdd_manager);
    long num_mem_bytes = Cudd_ReadMemoryInUse(m_bdd_manager);
    std::string result =
        "CUDD stats: #nodes: " + std::to_string(num_nodes) + " #peak nodes: " + std::to_string(num_peak_nodes) +
        " #reorderings: " + std::to_string(num_reorderings) + " #memory bytes: " + std::to_string(num_mem_bytes);

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
        var = Cudd_bddIthVar(m_bdd_manager, std::abs(clause[j]));

        if (clause[j] > 0) {
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

void bdd_manager::add_exactly_one_constraint(std::vector<int> &variables) {
    // order variables by the variable order
    // variable in the lowest layer comes first
    // variable in the highest layer comes last
    std::vector<std::pair<int, int>> layer_zipped_vars;
    for(int i = 0; i < variables.size(); i++){
        int var = variables[i];
        int layer = Cudd_ReadPerm(m_bdd_manager, var);
        layer_zipped_vars.push_back(std::make_pair(layer, var));
    }
    // sort it descending by layer
    std::sort(layer_zipped_vars.begin(), layer_zipped_vars.end(), std::greater<>());
    std::vector<int> ordered_variables;
    for(int i = 0; i < layer_zipped_vars.size(); i++) {
        ordered_variables.push_back(layer_zipped_vars[i].second);
    }

    DdNode *false_node = Cudd_ReadLogicZero(m_bdd_manager);
    DdNode *exact_one_true = Cudd_ReadOne(m_bdd_manager);
    Cudd_Ref(exact_one_true);
    DdNode *exact_zero_true = Cudd_ReadLogicZero(m_bdd_manager);
    Cudd_Ref(exact_zero_true);

    // from bottom to top, ignoring the topmost variable
    for (int i = 0; i < ordered_variables.size() - 1; i++) {
        // create nodes for new layer
        // I would have liked to use cuddUniqueInter here. It searches the unique table for the given node
        // and creates a new one if it does not exist
        // Unfortunately i was getting segfaults when using it
        DdNode *var_i = Cudd_bddIthVar(m_bdd_manager, ordered_variables[i]);
        DdNode *new_exact_one_true = Cudd_bddIte(m_bdd_manager, var_i, false_node, exact_one_true);
        Cudd_Ref(new_exact_one_true);
        DdNode *new_exact_zero_true = Cudd_bddIte(m_bdd_manager, var_i, exact_one_true, exact_zero_true);
        Cudd_Ref(new_exact_zero_true);

        // clean up old nodes
        Cudd_RecursiveDeref(m_bdd_manager, exact_one_true);
        Cudd_RecursiveDeref(m_bdd_manager, exact_zero_true);
        exact_one_true = new_exact_one_true;
        exact_zero_true = new_exact_zero_true;
    }

    // build the root node for the last variable
    DdNode *var_last = Cudd_bddIthVar(m_bdd_manager, ordered_variables[ordered_variables.size() - 1]);
    DdNode *constraint_root_node = Cudd_bddIte(m_bdd_manager, var_last, exact_one_true, exact_zero_true);
    Cudd_Ref(constraint_root_node);
    Cudd_RecursiveDeref(m_bdd_manager, exact_one_true);
    Cudd_RecursiveDeref(m_bdd_manager, exact_zero_true);

    // conjoin the root node for the exact one constraint with the root node of the bdd
    DdNode *tmp = Cudd_bddAnd(m_bdd_manager, m_root_node, constraint_root_node);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    Cudd_RecursiveDeref(m_bdd_manager, constraint_root_node);
    m_root_node = tmp;
}

void bdd_manager::hack_back_rocket_method() {
    DdNode *exact_one_true = Cudd_ReadOne(m_bdd_manager);
    Cudd_Ref(exact_one_true);
    DdNode *exact_zero_true = Cudd_ReadLogicZero(m_bdd_manager);
    Cudd_Ref(exact_zero_true);

    DdNode *var_i = Cudd_bddIthVar(m_bdd_manager, 3);
    Cudd_Ref(var_i);
    DdNode *ite = Cudd_bddIte(m_bdd_manager, var_i, exact_zero_true, exact_one_true);
    Cudd_Ref(ite);
    Cudd_RecursiveDeref(m_bdd_manager, var_i);
    Cudd_RecursiveDeref(m_bdd_manager, exact_one_true);
    Cudd_RecursiveDeref(m_bdd_manager, exact_zero_true);

    // conjoin the clause with the root node
    DdNode *temp = Cudd_bddAnd(m_bdd_manager, m_root_node, ite);
    Cudd_Ref(temp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    Cudd_RecursiveDeref(m_bdd_manager, ite);
    m_root_node = temp;
}

void bdd_manager::set_variable_order(std::vector<int> &variable_order){
    LOG_MESSAGE(log_level::info) << "Setting variable order msize: " << Cudd_ReadSize(m_bdd_manager) << " osize: " << variable_order.size();
    int* order = &variable_order[0];
    Cudd_ShuffleHeap(m_bdd_manager, order);
}

// This method returns the variable permutation
// the ith entry of the return vector dictates what variable (index) resides in the ith layer of the bdd
std::vector<int> bdd_manager::get_variable_order() {
    std::vector<int> layer_to_variable_index(m_num_variables, -1);
    for (int i = 0; i <= m_num_variables; i++) {
        // tells us at what layer the var resides in
        int layer_of_variable_i = Cudd_ReadPerm(m_bdd_manager, i);
        if (layer_of_variable_i == -1) {  // variable does not exist
            continue;
        }
        layer_to_variable_index[layer_of_variable_i] = i;
    }
    return layer_to_variable_index;
}

// TODO: find out what swapvars, bddpermute does
// what is a variable map?