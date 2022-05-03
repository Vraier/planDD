#include <stdio.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "cnf_encoder.h"
#include "dd_builder.h"
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

    if (options.m_values.encode_cnf) {
        sas_parser parser(options.m_values.sas_file);
        if(parser.start_parsing() == -1){
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);
        encoder.write_cnf_to_file(options.m_values.cnf_file, clauses);

        return 0;
    }

    if (options.m_values.build_bdd) {
        sas_parser parser(options.m_values.sas_file);
        if(parser.start_parsing() == -1){
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);

        bdd_manager builder;
        dd_builder::construct_dd_linear_disjoint(builder, clauses, options.m_values.build_order, options.m_values.reverse_order);
        builder.print_bdd(clauses.get_num_variables());

        return 0;
    }

    
    if (options.m_values.build_sdd) {
        sas_parser parser(options.m_values.sas_file);
        if(parser.start_parsing() == -1){
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);

        //encoder.write_cnf_to_file(options.m_values.cnf_file, clauses);
        sdd_manager builder(clauses.get_num_variables());
        LOG_MESSAGE(log_level::info) << "Start building sdd";

        dd_builder::construct_dd_linear_disjoint(builder, clauses, options.m_values.build_order, options.m_values.reverse_order);
        builder.print_sdd();

        return 0;
    }

    if (options.m_values.single_minisat) {
        sas_parser parser(options.m_values.sas_file);
        if(parser.start_parsing() == -1){
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);
        encoder.write_cnf_to_file(options.m_values.cnf_file, clauses);

        LOG_MESSAGE(log_level::info) << "Envoking minisat";
        std::string minisat_call =
            "../MiniSat_v1.14_linux " + options.m_values.cnf_file + " " + options.m_values.ass_file;
        std::system(minisat_call.c_str());

        std::vector<bool> assignment = encoder.parse_cnf_solution(options.m_values.ass_file);
        encoder.decode_cnf_solution(assignment, options.m_values.timesteps);
        return 0;
    }

    if (options.m_values.count_minisat) {
        // parse sas file
        sas_parser parser(options.m_values.sas_file);
        if(parser.start_parsing() == -1){
            LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
            return 0;
        }

        // encode it into cnf file
        cnf_encoder encoder(options.m_values, parser.m_sas_problem);
        planning_cnf::cnf clauses = encoder.encode_cnf(options.m_values.timesteps);
        encoder.write_cnf_to_file(options.m_values.cnf_file, clauses);

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
            clauses.add_clause(new_blocking_clause, planning_cnf::none, -1);
            encoder.write_cnf_to_file(options.m_values.cnf_file, clauses);
            num_solutions++;
        }

        LOG_MESSAGE(log_level::debug) << "Found a total of " << num_solutions << " solutions";

        return 0;
    }

    // interpret_sat_solution("minisat_out", 11);

    /*
    cnf = std::vector<std::vector<int>>();
    std::vector<int> c1, c2, c3, c4;
    c1.push_back(1); c1.push_back(2); c1.push_back(3);
    //c2.push_back(2);
    //c3.push_back(3);
    c4.push_back(4); c4.push_back(5);
    cnf.push_back(c1);
    //cnf.push_back(c2);
    //cnf.push_back(c3);
    cnf.push_back(c4);
    */
    return 0;
}