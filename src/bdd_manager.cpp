#include "logging.h"

#include "bdd_manager.h"

bdd_manager::bdd_manager() {
    m_bdd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    Cudd_AutodynEnable(m_bdd_manager, CUDD_REORDER_SIFT);
    //Cudd_SetMaxCacheHard(m_bdd_manager, 62500000);

    m_root_node = Cudd_ReadOne(m_bdd_manager);
    Cudd_Ref(m_root_node);
}

bdd_manager::~bdd_manager() { 
    Cudd_Quit(m_bdd_manager); 
}

// Print a bdd summary
void bdd_manager::print_bdd(int num_variables) {
    // TODO: i have to find the values that interes me and that i want to output here. for example: time spent
    // reordering, time spend, conjoining....

    // printf("Ddm_bdd_manager nodes: %ld | ",
    //        Cudd_ReadNodeCount(m_bdd_manager)); /*Reports the number of live nodes in BDDs and ADDs*/
    // printf("Ddm_bdd_manager vars: %d | ",
    //        Cudd_ReadSize(m_bdd_manager)); /*Returns the number of BDD variables in existence*/
    // printf("Ddm_bdd_manager reorderings: %d | ",
    //        Cudd_ReadReorderings(m_bdd_manager)); /*Returns the number of times reordering has occurred*/
    // printf("Ddm_bdd_manager memory: %ld \n",
    //        Cudd_ReadMemoryInUse(m_bdd_manager)); /*Returns the memory in use by the m_bdd_manager measured in bytes*/
    // printf("Ddm_bdd_manager paths: %f \n", Cudd_CountPathsToNonZero(m_root_node));
    LOG_MESSAGE(log_level::info) << "Number of nodes: " << Cudd_DagSize(m_root_node) << ", Number of solutions: "
                                 << Cudd_CountMinterm(m_bdd_manager, m_root_node, num_variables);
    LOG_MESSAGE(log_level::info) << "Printing CUDD statistics...";
    FILE** fout = &stdout;
    Cudd_PrintInfo(m_bdd_manager, *fout);
    // Cudd_PrintSummary(m_bdd_manager, m_root_node, num_variables, 2);
}

// Writes a dot file representing the argument DDs
void bdd_manager::write_bdd_to_dot_file(DdNode *bdd, std::string filename) {
    DdNode *add = Cudd_BddToAdd(m_bdd_manager, bdd);
    FILE *outfile;  // output file pointer for .dot file
    outfile = fopen(filename.c_str(), "w");
    DdNode **ddnodearray = (DdNode **)malloc(sizeof(DdNode *));  // initialize the function array
    ddnodearray[0] = add;
    Cudd_DumpDot(m_bdd_manager, 1, ddnodearray, NULL, NULL, outfile);  // dump the function to .dot file
    free(ddnodearray);
    fclose(outfile);
}

/*
// Returns a DdNode representing the cnf with a reference count of 1
DdNode *bdd_manager::construct_bdd_from_cnf(std::vector<std::vector<int>> &cnf) {
    for (int i = 0; i < cnf.size(); i++) {
        if ((i + 1) % 100 == 0) {
            LOG_MESSAGE(log_level::trace) << "Building clause " << i + 1 << " out of " << cnf.size();
        }

        std::vector<int> clause = cnf[i];

        conjoin_clause(clause);
    }
    return m_root_node;
}*/

/*
// Returns a DdNode representing the cnf with a reference count of 1
DdNode *bdd_manager::construct_bdd_from_cnf_binary(std::vector<std::vector<int>> &cnf) {
    DdNode *conjunction = Cudd_ReadOne(m_bdd_manager);
    Cudd_Ref(conjunction);

    std::vector<DdNode *> current_clauses;
    std::vector<DdNode *> new_clauses;

    LOG_MESSAGE(log_level::info) << "Constructing the bottom most layer of clauses";
    // build the bottom most layer of clauses
    for (int i = 0; i < cnf.size(); i++) {
        std::vector<int> clause = cnf[i];

        // build the disjunction of the literals in the clause
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

        current_clauses.push_back(disjunction);
    }

    // the total number of clauses halfes in size for each step
    while (current_clauses.size() > 1) {
        LOG_MESSAGE(log_level::info) << "Currently merging " << current_clauses.size() << " clauses";
        for (int i = 0; i < current_clauses.size() - 1; i += 2) {
            if (current_clauses.size() < 300 && ((i % 10) == 0)) {
                LOG_MESSAGE(log_level::info) << "Building clause " << i + 1 << " out of " << current_clauses.size();
            }
            // TODO: this can probably be optimizes by not constructing the logical 1. but it does not matter that much?
            DdNode *clauses1 = current_clauses[i];
            DdNode *clauses2 = current_clauses[i + 1];
            DdNode *conjunction = Cudd_ReadOne(m_bdd_manager);
            Cudd_Ref(conjunction);
            DdNode *tmp;

            // add both nodes together
            tmp = Cudd_bddAnd(m_bdd_manager, conjunction, clauses1);
            Cudd_Ref(tmp);
            Cudd_RecursiveDeref(m_bdd_manager, conjunction);
            Cudd_RecursiveDeref(m_bdd_manager, clauses1);
            conjunction = tmp;

            tmp = Cudd_bddAnd(m_bdd_manager, conjunction, clauses2);
            Cudd_Ref(tmp);
            Cudd_RecursiveDeref(m_bdd_manager, conjunction);
            Cudd_RecursiveDeref(m_bdd_manager, clauses2);
            conjunction = tmp;

            new_clauses.push_back(conjunction);
        }

        if ((current_clauses.size() % 2) == 1) {
            // if we have an odd number of clauses, we have to add the last clause
            new_clauses.push_back(current_clauses[current_clauses.size() - 1]);
        }

        current_clauses = new_clauses;
        new_clauses = std::vector<DdNode *>();
    }

    return current_clauses[0];
}*/

void bdd_manager::conjoin_clause(std::vector<int> &clause) {
    // build the disjunction of the literals in the clause
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
}