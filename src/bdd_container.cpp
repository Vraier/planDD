#include "logging.h"
#include "dd_builder_conjoin_order.h"

#include "bdd_container.h"

bdd_container::bdd_container(int num_bdds, int num_variables) {
    // add one more variable to account for variable with index 0
    m_num_variables = num_variables;

    m_bdd_manager = Cudd_Init(m_num_variables, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    Cudd_AutodynEnable(m_bdd_manager, CUDD_REORDER_SIFT);

    // Cudd_SetMaxCacheHard(m_bdd_manager, 62500000);

    // construct correct abount of root nodes
    m_root_nodes = std::vector<DdNode *>(num_bdds);
    for (int i = 0; i < num_bdds; i++) {
        // force var with index 0 to be true
        m_root_nodes[i] = Cudd_bddIthVar(m_bdd_manager, 0);
        Cudd_Ref(m_root_nodes[i]);
    }
}

bdd_container::~bdd_container() {
    // check if all nodes were dereferenced corretly (no memory leaks (:)
    for (int i = 0; i < m_root_nodes.size(); i++) {
        Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[i]);
    }
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
    LOG_MESSAGE(log_level::info) << "Finished reducing heap. New size: " << Cudd_ReadNodeCount(m_bdd_manager);
}

void bdd_container::print_bdd_info() {
    LOG_MESSAGE(log_level::info) << "Printing CUDD statistics...";
    LOG_MESSAGE(log_level::info) << "Number of nodes: " << Cudd_DagSize(m_root_nodes[0])
                                 << ", Num Variables: " << m_num_variables << ", Number of solutions: "
                                 << Cudd_CountMinterm(m_bdd_manager, m_root_nodes[0], m_num_variables + 1);
    FILE **fout = &stdout;
    Cudd_PrintInfo(m_bdd_manager, *fout);
}

std::vector<std::vector<bool>> bdd_container::list_minterms(int max) {
    LOG_MESSAGE(log_level::info) << "Start finding minterms";

    DdNode *curr_node = m_root_nodes[0];
    Cudd_Ref(curr_node);
    std::vector<std::vector<bool>> assignments;

    DdNode **nodes = new DdNode *[m_num_variables];
    for (int i = 0; i < m_num_variables; i++) {
        nodes[i] = Cudd_bddIthVar(m_bdd_manager, i);
    }

    while (curr_node != Cudd_ReadLogicZero(m_bdd_manager) && assignments.size() < max) {
        DdNode *minterm = Cudd_bddPickOneMinterm(m_bdd_manager, curr_node, nodes, m_num_variables);
        Cudd_Ref(minterm);
        // construct new assignment vector
        std::vector<bool> new_assignment;
        for (int i = 0; i < m_num_variables; i++) {
            new_assignment.push_back(Cudd_bddLeq(m_bdd_manager, minterm, nodes[i]) == 0 ? false : true);
        }

        DdNode *new_curr = Cudd_bddAnd(m_bdd_manager, curr_node, Cudd_Not(minterm));
        Cudd_Ref(new_curr);
        Cudd_RecursiveDeref(m_bdd_manager, curr_node);
        Cudd_RecursiveDeref(m_bdd_manager, minterm);
        curr_node = new_curr;

        assignments.push_back(new_assignment);
    }

    Cudd_RecursiveDeref(m_bdd_manager, curr_node);
    delete[] nodes;

    LOG_MESSAGE(log_level::info) << "Finished finding minterms";
    return assignments;
}

std::string bdd_container::get_short_statistics(int bdd_index) {
    long num_nodes = Cudd_DagSize(m_root_nodes[bdd_index]);
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
    DdNode *add = Cudd_BddToAdd(m_bdd_manager, m_root_nodes[0]);
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

void bdd_container::clear_dd(int bdd_index) {
    Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[bdd_index]);
    m_root_nodes[bdd_index] = Cudd_bddIthVar(m_bdd_manager, 0);
    Cudd_Ref(m_root_nodes[bdd_index]);
}

bool bdd_container::is_constant_false(int bdd_index) {
    return m_root_nodes[bdd_index] == Cudd_ReadLogicZero(m_bdd_manager);
}

void bdd_container::add_clause_primitive(std::vector<int> &clause, int bdd_index) {
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
    tmp = Cudd_bddAnd(m_bdd_manager, m_root_nodes[bdd_index], disjunction);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[bdd_index]);
    Cudd_RecursiveDeref(m_bdd_manager, disjunction);
    m_root_nodes[bdd_index] = tmp;
}

void bdd_container::add_exactly_one_primitive(std::vector<int> &variables, int bdd_index) {
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
    DdNode *tmp = Cudd_bddAnd(m_bdd_manager, m_root_nodes[bdd_index], constraint_root_node);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[bdd_index]);
    Cudd_RecursiveDeref(m_bdd_manager, constraint_root_node);
    m_root_nodes[bdd_index] = tmp;
}

void bdd_container::add_dnf_primitive(std::vector<std::vector<int>> &dnf, int bdd_index) {
    // build the disjunction of the literals in the clause
    DdNode *var, *tmp;
    DdNode *disjunction = Cudd_ReadLogicZero(m_bdd_manager);
    Cudd_Ref(disjunction);

    for (int i = 0; i < dnf.size(); i++) {
        DdNode *conjunction = Cudd_ReadOne(m_bdd_manager);
        Cudd_Ref(conjunction);

        for (int j = 0; j < dnf[i].size(); j++) {
            var = Cudd_bddIthVar(m_bdd_manager, std::abs(dnf[i][j]));
            if (dnf[i][j] > 0) {
                tmp = Cudd_bddAnd(m_bdd_manager, var, conjunction);
            } else {
                tmp = Cudd_bddAnd(m_bdd_manager, Cudd_Not(var), conjunction);
            }
            Cudd_Ref(tmp);
            Cudd_RecursiveDeref(m_bdd_manager, conjunction);
            conjunction = tmp;
        }

        tmp = Cudd_bddOr(m_bdd_manager, disjunction, conjunction);
        Cudd_Ref(tmp);
        Cudd_RecursiveDeref(m_bdd_manager, disjunction);
        Cudd_RecursiveDeref(m_bdd_manager, conjunction);
        disjunction = tmp;
    }

    // conjoin the clause with the root node
    tmp = Cudd_bddAnd(m_bdd_manager, m_root_nodes[bdd_index], disjunction);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[bdd_index]);
    Cudd_RecursiveDeref(m_bdd_manager, disjunction);
    m_root_nodes[bdd_index] = tmp;
}

void bdd_container::hack_back_rocket_method() { return; }

void bdd_container::disable_reordering(){
    Cudd_AutodynDisable(m_bdd_manager);
}
void bdd_container::enable_reordering(){
    Cudd_AutodynEnable(m_bdd_manager, CUDD_REORDER_SIFT);
}

void bdd_container::set_variable_group(int low, int size){
    Cudd_MakeTreeNode(m_bdd_manager, low, size, MTR_DEFAULT);
}

void bdd_container::set_variable_order(std::vector<int> &variable_order) {
    LOG_MESSAGE(log_level::info) << "Setting variable order. manager size: " << Cudd_ReadSize(m_bdd_manager)
                                 << " order size: " << variable_order.size();
    int *order = variable_order.data();
    Cudd_ShuffleHeap(m_bdd_manager, order);
}

std::vector<int> bdd_container::get_variable_order() {
    std::vector<int> index_to_layer(m_num_variables, -1);
    for (int i = 0; i <= m_num_variables; i++) {
        // tells us at what layer the var resides in
        int layer_of_variable_i = Cudd_ReadPerm(m_bdd_manager, i);
        if (layer_of_variable_i == -1) {  // variable does not exist
            continue;
        }
        index_to_layer[i] = layer_of_variable_i;
    }
    return index_to_layer;
}

void bdd_container::copy_and_conjoin_bdd_from_another_container(bdd_container &copy_from) {
    LOG_MESSAGE(log_level::info) << "Copying bdd to another";
    // transfer the bdd from one manager to another
    DdNode *copied_bdd = Cudd_bddTransfer(copy_from.m_bdd_manager, m_bdd_manager, copy_from.m_root_nodes[0]);
    Cudd_Ref(copied_bdd);

    LOG_MESSAGE(log_level::info) << "Conjoining the copied bdd";
    // conjoin the transferd bdd with the root node
    DdNode *tmp = Cudd_bddAnd(m_bdd_manager, m_root_nodes[0], copied_bdd);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[0]);
    Cudd_RecursiveDeref(m_bdd_manager, copied_bdd);

    m_root_nodes[0] = tmp;
}

void bdd_container::permute_variables(std::vector<int> &permutation, int source_bdd, int destination_bdd) {
    int num_variables = permutation.size();
    if (num_variables != Cudd_ReadSize(m_bdd_manager)) {
        LOG_MESSAGE(log_level::error) << "Cant permutate variables. Permutation size: " << num_variables
                                      << ", manager size: " << Cudd_ReadSize(m_bdd_manager);
        return;
    }
    LOG_MESSAGE(log_level::info) << "Permuating " << num_variables << " variables";

    // now perform the swapping
    DdNode *temp_node = Cudd_bddPermute(m_bdd_manager, m_root_nodes[source_bdd], permutation.data());
    Cudd_Ref(temp_node);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[destination_bdd]);
    m_root_nodes[destination_bdd] = temp_node;
}

void bdd_container::conjoin_two_dds(int bbd_a, int bdd_b, int bdd_result) {
    LOG_MESSAGE(log_level::info) << "Conjoining two bdds";
    DdNode *tmp = Cudd_bddAnd(m_bdd_manager, m_root_nodes[bbd_a], m_root_nodes[bdd_b]);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(m_bdd_manager, m_root_nodes[bdd_result]);
    m_root_nodes[bdd_result] = tmp;
}

std::map<int, int> bdd_container::get_variable_order_for_single_step(
    std::map<planning_logic::tagged_variable, int> &variable_map) {
    std::map<int, int> layer_to_index;
    std::map<int, int> consolidated_index_to_layer;  // same as above but with no gaps in the layers
    // dummy 0 var has alway should be at layer 0
    layer_to_index[0] = 0;

    for (std::map<planning_logic::tagged_variable, int>::iterator iter = variable_map.begin();
         iter != variable_map.end(); ++iter) {
        planning_logic::tagged_variable tagged_var = iter->first;
        int index = iter->second;

        int t = std::get<1>(tagged_var);
        if (t != 0) {
            continue;
        }

        int layer = Cudd_ReadPerm(m_bdd_manager, index);
        layer_to_index[layer] = index;
    }

    int new_layer = 0;
    for (std::map<int, int>::iterator iter = layer_to_index.begin(); iter != layer_to_index.end(); ++iter) {
        // int layer = iter->first;
        int index = iter->second;
        consolidated_index_to_layer[index] = new_layer;
        new_layer++;

        // std::cout << "index: " << index << " layer: " << layer << " newLayer: " << new_layer-1 << std::endl;
    }

    return consolidated_index_to_layer;
}

std::vector<int> bdd_container::extend_variable_order_to_all_steps(
    std::map<planning_logic::tagged_variable, int> &variable_map, std::map<int, int> &single_step_order) {
    LOG_MESSAGE(log_level::info) << "Extending Variable order to multiple timesteps. single step size: "
                                 << single_step_order.size();

    int num_variables_in_one_timestep = single_step_order.size();
    std::map<int, int> layer_to_index;
    std::vector<int> result_index_to_layer_map;

    for (std::map<planning_logic::tagged_variable, int>::iterator iter = variable_map.begin();
         iter != variable_map.end(); ++iter) {
        planning_logic::tagged_variable tagged_var = iter->first;
        int index = iter->second;
        int t = std::get<1>(tagged_var);

        // create a var tuple that represents the variable in timestep 0 and calculates its layer in the single step bdd
        planning_logic::tagged_variable zero_step_tagged_var = tagged_var;
        std::get<1>(zero_step_tagged_var) = 0;
        int index_of_zero_step_var = variable_map[zero_step_tagged_var];
        int layer_in_zero_step = single_step_order[index_of_zero_step_var];
        int layer_in_t_step = (t * num_variables_in_one_timestep) + layer_in_zero_step;

        layer_to_index[layer_in_t_step] = index;
    }

    // consolidate the layers
    result_index_to_layer_map = std::vector<int>(variable_map.size() + 1);  // +1 for the dummy 0 var
    result_index_to_layer_map[0] = 0;
    int new_layer = 1;
    for (std::map<int, int>::iterator iter = layer_to_index.begin(); iter != layer_to_index.end(); ++iter) {
        // int layer = iter->first;
        int index = iter->second;
        result_index_to_layer_map[index] = new_layer;
        new_layer++;

        // std::cout << "index: " << index << " layer: " << layer << " newLayer: " << new_layer-1 << std::endl;
    }

    LOG_MESSAGE(log_level::info) << "Extendet size is: " << result_index_to_layer_map.size();
    return result_index_to_layer_map;
}