#include <stdio.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "cnf_encoder.h"
#include "dd_builder.h"
#include "dd_builder_variable_order.h"
#include "logging.h"
#include "options.h"
#include "sas_parser.h"
#include "sdd_manager.h"
#include "bdd_manager.h"  // always include this last >:(


int main(int argc, char *argv[]) {
    initialize_logging();

    option_parser options;
    options.parse_command_line(argc, argv);

    options.print_variable_map();

    if (!options.check_validity()) {
        options.print_help();
        return 0;
    }

    if (options.m_values.hack_debug) {
        LOG_MESSAGE(log_level::info) << "You unlocked full control. Good luck modifying the source code";

        int num_variables = options.m_values.timesteps;
        std::vector<int> constraint;
        for (int i = 1; i <= num_variables; i++) {
            constraint.push_back(i);
        }

        bdd_manager builder(num_variables);

        LOG_MESSAGE(log_level::debug) << "Starting hack back rocket";
        builder.add_exactly_one_constraint(constraint);
        LOG_MESSAGE(log_level::debug) << "Landed Hack back rocket";

        LOG_MESSAGE(log_level::debug) << builder.get_short_statistics();
        builder.print_bdd();

        builder.write_bdd_to_dot_file("exactly_one_constraint.dot");
    }

    if (options.m_values.cnf_to_bdd) {
        std::tuple<int, int, std::vector<planning_cnf::clause>> cnf_data =
            planning_cnf::cnf::parse_cnf_file_to_clauses(options.m_values.cnf_file);
        int num_variables = std::get<0>(cnf_data);
        // int num_clauses = std::get<1>(cnf_data);
        std::vector<planning_cnf::clause> clauses = std::get<2>(cnf_data);
        bdd_manager builder(num_variables);

        for (planning_cnf::clause c : clauses) {
            builder.conjoin_clause(c);
        }

        std::vector<int> var_order = builder.get_variable_order();
        for (int i = 0; i < var_order.size(); i++) {
            std::cout << "At index " << i << ": " << var_order[i] << std::endl;
        }

        builder.write_bdd_to_dot_file("befor_reorder.dot");

        std::cout << "Order after reducing heap" << std::endl;

        builder.reduce_heap();
        var_order = builder.get_variable_order();
        for (int i = 0; i < var_order.size(); i++) {
            std::cout << "At index " << i << ": " << var_order[i] << std::endl;
        }

        LOG_MESSAGE(log_level::debug) << builder.get_short_statistics();
        builder.print_bdd();

        builder.write_bdd_to_dot_file("after_reorder.dot");
    }

    if (options.m_values.encode_cnf) {
        sas_parser parser(options.m_values.sas_file);
        if (parser.start_parsing() == -1) {
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);
        clauses.write_to_file(options.m_values.cnf_file);

        return 0;
    }

    if (options.m_values.build_bdd) {
        sas_parser parser(options.m_values.sas_file);
        if (parser.start_parsing() == -1) {
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);

        std::vector<int> var_order = variable_order::order_variables(clauses, options.m_values.variable_order,
                                                                     options.m_values.goal_variables_first,
                                                                     options.m_values.initial_state_variables_first);
        bdd_manager builder(clauses.get_num_variables());
        builder.set_variable_order(var_order);

        
        //std::vector<int> builder_order = builder.get_variable_order();
        //for(int i = 0; i < builder_order.size(); i++){
        //    std::cout << "Layer " << i << ", " << builder_order[i] << ": " <<
        //encoder.decode_cnf_variable(builder_order[i]) << std::endl;
        //}
        

        dd_builder::construct_dd_linear_disjoint(builder, clauses, options.m_values.build_order,
                                                 options.m_values.reverse_order);

        //std::vector<int> builder_order = builder.get_variable_order();
        //for(int i = 0; i < builder_order.size(); i++){
        //    std::cout << "Layer " << i << ", " << builder_order[i] << ": " <<
        //encoder.decode_cnf_variable(builder_order[i]) << std::endl;
        //}

        builder.print_bdd();
        return 0;
    }

    if (options.m_values.build_sdd) {
        sas_parser parser(options.m_values.sas_file);
        if (parser.start_parsing() == -1) {
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);

        // encoder.write_cnf_to_file(options.m_values.cnf_file, clauses);
        sdd_manager builder(clauses.get_num_variables());
        LOG_MESSAGE(log_level::info) << "Start building sdd";

        dd_builder::construct_dd_linear_disjoint(builder, clauses, options.m_values.build_order,
                                                 options.m_values.reverse_order);
        builder.print_sdd();

        return 0;
    }

    if (options.m_values.single_minisat) {
        sas_parser parser(options.m_values.sas_file);
        if (parser.start_parsing() == -1) {
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);
        clauses.write_to_file(options.m_values.cnf_file);

        LOG_MESSAGE(log_level::info) << "Envoking minisat";
        std::string minisat_call =
            "../MiniSat_v1.14_linux " + options.m_values.cnf_file + " " + options.m_values.ass_file;
        std::system(minisat_call.c_str());

        std::vector<bool> assignment = encoder.parse_cnf_solution(options.m_values.ass_file);
        encoder.decode_cnf_solution(assignment);
        return 0;
    }

    if (options.m_values.count_minisat) {
        // parse sas file
        sas_parser parser(options.m_values.sas_file);
        if (parser.start_parsing() == -1) {
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        // encode it into cnf file
        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);
        clauses.write_to_file(options.m_values.cnf_file);

        std::vector<bool> current_assignment;
        std::vector<bool> old_assignment;
        int num_solutions = 0;
        while (true) {
            // envoke minisat
            LOG_MESSAGE(log_level::debug) << "Envoking minisat, found " << num_solutions << " so far";
            std::string minisat_call =
                "../MiniSat_v1.14_linux " + options.m_values.cnf_file + " " + options.m_values.ass_file;
            std::system(minisat_call.c_str());
            current_assignment = encoder.parse_cnf_solution(options.m_values.ass_file);

            // stop if no new solution was found
            if (current_assignment.size() == 0) break;

// print difference between both assignments
#if 0
            if(current_assignment.size() > 0 && old_assignment.size() > 0){
                encoder.compare_assignments(current_assignment, old_assignment);
            }
            old_assignment = current_assignment;
            encoder.decode_cnf_solution(current_assignment, options.m_values.timesteps);
#endif

            // update the clauses with a new blockng one to prevent exactly the found solution
            std::vector<int> new_blocking_clause;
            for (int i = 1; i < current_assignment.size(); i++) {
                // value at index 0 has no meaning for sat solver (there is no variable 1)
                // add the negated variable (because of boolean algebra)
                new_blocking_clause.push_back(current_assignment[i] ? -i : i);
            }
            clauses.add_clause(new_blocking_clause, planning_cnf::none_clause, -1);
            clauses.write_to_file(options.m_values.cnf_file);
            num_solutions++;
        }

        LOG_MESSAGE(log_level::debug) << "Found a total of " << num_solutions << " solutions";

        return 0;
    }
    return 0;
}