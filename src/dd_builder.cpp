#include "dd_builder.h"

#include "dd_builder_conjoin_order.h"
#include "logging.h"

using namespace planning_cnf;

namespace dd_builder {

void construct_dd_linear_disjoint(dd_buildable &dd, cnf &cnf, std::string build_order, bool reversed) {
    LOG_MESSAGE(log_level::info) << "Construction DD with the following disjoint_order: " << build_order;

    if (!conjoin_order::is_valid_build_order_string(build_order)) {
        LOG_MESSAGE(log_level::error) << "Can't build the following order " << build_order;
        return;
    }

    // categorize clauses
    std::map<clause_tag, std::vector<std::vector<clause>>> tagged_clauses = conjoin_order::categorize_clauses(cnf);

    // sort the clauses (disjoint and interleaved)
    std::vector<clause> sorted_clauses = conjoin_order::sort_clauses(cnf, build_order, tagged_clauses);

    // this reverses the order of the clauses. It allows the variables with the highes timesteps to be conjoined first
    if (reversed) {
        LOG_MESSAGE(log_level::info) << "Reversing order of the clauses";
        std::reverse(sorted_clauses.begin(), sorted_clauses.end());
    }

    // conjoin the clauses in the correct order
    int percent = 0;
    for (int i = 0; i < sorted_clauses.size(); i++) {
        // for every clause in that timestep
        dd.conjoin_clause(sorted_clauses[i]);

        int new_percent = (100*(i+1))/sorted_clauses.size();
        if (new_percent > percent) {
            percent = new_percent;
            LOG_MESSAGE(log_level::info) << "Conjoined " << percent << "% of all clauses";
        }
    }

    LOG_MESSAGE(log_level::info) << "Finished constructing DD";
}
}  // namespace dd_builder