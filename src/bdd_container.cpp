#include "logging.h"
#include "dd_builder_conjoin_order.h"

#include "bdd_container.h"

bdd_container::bdd_container(int num_variables) {
    // add one more variable to account for variable with index 0
    m_num_variables = num_variables + 1;

    m_bdd_manager = Cudd_Init(m_num_variables, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    Cudd_AutodynEnable(m_bdd_manager, CUDD_REORDER_SIFT);

    // Cudd_SetMaxCacheHard(m_bdd_manager, 62500000);

    // force var with index 0 to be true
    m_root_node = Cudd_bddIthVar(m_bdd_manager, 0);
    Cudd_Ref(m_root_node);
}

bdd_container::~bdd_container() {
    // check if all nodes were dereferenced corretly (no memory leaks (:)
    Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    int num_zero_reference_nodes = Cudd_CheckZeroRef(m_bdd_manager);
    if (num_zero_reference_nodes != 0) {
        LOG_MESSAGE(log_level::warning) << "#Nodes with non-zero reference count (should be 0): "
                                        << Cudd_CheckZeroRef(m_bdd_manager);
    }

    Cudd_Quit(m_bdd_manager);
}

void bdd_container::reduce_heap() {
    LOG_MESSAGE(log_level::info) << "Start reducing heap manually";
    Cudd_ReduceHeap(m_bdd_manager, CUDD_REORDER_SIFT_CONVERGE, 1);
    LOG_MESSAGE(log_level::info) << "Finished reducing heap";
}

void bdd_container::print_bdd_info() {
    LOG_MESSAGE(log_level::info) << "Printing CUDD statistics...";
    LOG_MESSAGE(log_level::info) << "Number of nodes: " << Cudd_DagSize(m_root_node) << ", Number of solutions: "
                                 << Cudd_CountMinterm(m_bdd_manager, m_root_node, m_num_variables);
    FILE **fout = &stdout;
    Cudd_PrintInfo(m_bdd_manager, *fout);
}

std::string bdd_container::get_short_statistics() {
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
void bdd_container::write_bdd_to_dot_file(std::string filename) {
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

void bdd_container::conjoin_clause(std::vector<int> &clause) {
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

void bdd_container::add_exactly_one_constraint(std::vector<int> &variables) {
    // order variables by the variable order
    // variable in the lowest layer comes first
    // variable in the highest layer comes last
    std::vector<std::pair<int, int>> layer_zipped_vars;
    for (int i = 0; i < variables.size(); i++) {
        int var = variables[i];
        int layer = Cudd_ReadPerm(m_bdd_manager, var);
        layer_zipped_vars.push_back(std::make_pair(layer, var));
    }
    // sort it descending by layer
    std::sort(layer_zipped_vars.begin(), layer_zipped_vars.end(), std::greater<>());
    std::vector<int> ordered_variables;
    for (int i = 0; i < layer_zipped_vars.size(); i++) {
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

void bdd_container::hack_back_rocket_method() {
    // DdNode *exact_one_true = Cudd_ReadOne(m_bdd_manager);
    // Cudd_Ref(exact_one_true);
    // DdNode *exact_zero_true = Cudd_ReadLogicZero(m_bdd_manager);
    // Cudd_Ref(exact_zero_true);

    bdd_container single_step(4);

    std::vector<int> constraint;
    for (int i = 1; i <= 4; i++) {
        constraint.push_back(i);
    }

    single_step.add_exactly_one_constraint(constraint);
    add_exactly_one_constraint(constraint);

    DdNode *from[4];
    DdNode *to[4];

    from[0] = Cudd_bddIthVar(m_bdd_manager, 1);
    from[1] = Cudd_bddIthVar(m_bdd_manager, 2);
    from[2] = Cudd_bddIthVar(m_bdd_manager, 3);
    from[3] = Cudd_bddIthVar(m_bdd_manager, 4);

    to[0] = Cudd_bddIthVar(m_bdd_manager, 5);
    to[1] = Cudd_bddIthVar(m_bdd_manager, 6);
    to[2] = Cudd_bddIthVar(m_bdd_manager, 7);
    to[3] = Cudd_bddIthVar(m_bdd_manager, 8);

    m_root_node = Cudd_bddSwapVariables(m_bdd_manager, m_root_node, from, to, 4);

    DdNode *copied_bdd = Cudd_bddTransfer(single_step.m_bdd_manager, m_bdd_manager, single_step.m_root_node);
    m_root_node = Cudd_bddAnd(m_bdd_manager, m_root_node, copied_bdd);

    // DdNode *var_i = Cudd_bddIthVar(m_bdd_manager, 3);
    // Cudd_Ref(var_i);
    // DdNode *ite = Cudd_bddIte(m_bdd_manager, var_i, exact_zero_true, exact_one_true);
    // Cudd_Ref(ite);
    // Cudd_RecursiveDeref(m_bdd_manager, var_i);
    // Cudd_RecursiveDeref(m_bdd_manager, exact_one_true);
    // Cudd_RecursiveDeref(m_bdd_manager, exact_zero_true);
    //
    //// conjoin the clause with the root node
    // DdNode *temp = Cudd_bddAnd(m_bdd_manager, m_root_node, ite);
    // Cudd_Ref(temp);
    // Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    // Cudd_RecursiveDeref(m_bdd_manager, ite);
    // m_root_node = temp;
}

void bdd_container::set_variable_order(std::vector<int> &variable_order) {
    LOG_MESSAGE(log_level::info) << "Setting variable order. manager size: " << Cudd_ReadSize(m_bdd_manager)
                                 << " order size: " << variable_order.size();
    int *order = &variable_order[0];
    Cudd_ShuffleHeap(m_bdd_manager, order);
}

std::vector<int> bdd_container::get_variable_order() {
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

DdNode *bdd_container::copy_bdd_to_other_container(bdd_container &copy_to) {

    DdNode *copied_bdd = Cudd_bddTransfer(m_bdd_manager, copy_to.m_bdd_manager, m_root_node);
    return copied_bdd;
}

void bdd_container::swap_variables_to_other_timestep(std::map<planning_logic::tagged_variable, int> &variable_map, int timestep_from, int timestep_to){

    // first i just calculate the mapping
    std::vector<int> indices_from, indices_to;
    for(std::map<planning_logic::tagged_variable, int>::iterator iter = variable_map.begin(); iter != variable_map.end(); ++iter) {
        planning_logic::tagged_variable tagged_var = iter->first;
        int var_index = iter->second;

        // since the variable map orders variables in the order: tag, timestep, index, value, i dont have to sort them when calculating the mapping (they are already sorted)
        int t = std::get<1>(tagged_var);
        if(t == timestep_from){
            indices_from.push_back(var_index);
        }
        if(t == timestep_to){
            indices_to.push_back(var_index);
        }
    }

    // TODO assert both arrays have same size
    // now i construct the necessary arrays for CUDD
    int variables_in_single_timestep = indices_from.size();
    DdNode** nodes_from = new DdNode*[variables_in_single_timestep];
    DdNode** nodes_to = new DdNode*[variables_in_single_timestep];

    for(int i = 0; i < variables_in_single_timestep; i++){
        nodes_from[i] = Cudd_bddIthVar(m_bdd_manager, indices_from[i]);
        nodes_to[i] = Cudd_bddIthVar(m_bdd_manager, indices_to[i]);
    }

    // now perform the swapping
    DdNode *temp_node = Cudd_bddSwapVariables(m_bdd_manager, m_root_node, nodes_from, nodes_to, variables_in_single_timestep);
    Cudd_Ref(temp_node);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_node);
    m_root_node = temp_node;

    delete[] nodes_from;
    delete[] nodes_to;
}

std::vector<int> bdd_container::get_variable_order_for_single_step(std::map<planning_logic::tagged_variable, int> &variable_map){

    std::map<int, int> index_to_layer;
    std::vector<int> variable_order; // maps var index to layer

    for(std::map<planning_logic::tagged_variable, int>::iterator iter = variable_map.begin(); iter != variable_map.end(); ++iter) {
        planning_logic::tagged_variable tagged_var = iter->first;
        int index = iter->second;

        int t = std::get<1>(tagged_var);
        if(t != 0) {
            continue;
        }

        int layer = Cudd_ReadPerm(m_bdd_manager, index);
        index_to_layer[index] = layer;
    }

    for(std::map<int, int>::iterator iter = index_to_layer.begin(); iter != index_to_layer.end(); ++iter) {
        int layer = iter->second;
        variable_order.push_back(layer);
    }

    return variable_order;
}

std::vector<int> bdd_container::extend_variable_order_to_all_steps(std::map<planning_logic::tagged_variable, int> &variable_map, int timesteps, std::vector<int> &single_step_order){

    int num_variables_in_one_timestep = single_step_order.size();
    std::vector<int> total_order(num_variables_in_one_timestep * (1+timesteps)); // maps var index to layer

    for(std::map<planning_logic::tagged_variable, int>::iterator iter = variable_map.begin(); iter != variable_map.end(); ++iter) {
        planning_logic::tagged_variable tagged_var = iter->first;
        int index = iter->second;
        int t = std::get<1>(tagged_var);

        // create a var tuple that represents the variable in timestep 0 and calculates its layer in the single step bdd
        planning_logic::tagged_variable zero_step_tagged_var = tagged_var;
        std::get<1>(zero_step_tagged_var) = 0; 
        int index_of_zero_step_var = variable_map[zero_step_tagged_var];
        int layer_in_zero_step = single_step_order[index_of_zero_step_var];

        // calculate the corret layer of the var in the total order
        total_order[index] = (t*num_variables_in_one_timestep) + layer_in_zero_step;
    }

    return total_order;
}