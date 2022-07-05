#include "planDD.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>

#include "logging.h"
#include "cnf_encoder.h"
#include "sas_parser.h"
#include "sdd_container.h"
#include "dd_builder.h"
#include "dd_builder_variable_order.h"
#include "dd_builder_conjoin_order.h"
#include "bdd_container.h"
#include "plan_to_cnf_map.h"

int main(int argc, char *argv[]) {
    // start logging
    initialize_logging();

    // parse command line options
    option_parser options;
    options.parse_command_line(argc, argv);
    options.print_variable_map();
    if (!options.check_validity()) {
        options.print_help();
        return 0;
    }

    // branch into correct program mode
    if (options.m_values.hack_debug) {
        return planDD::hack_debug(options.m_values);
    }

    if (options.m_values.build_bdd) {
        return planDD::build_bdd(options.m_values);
    }

    if (options.m_values.build_bdd_by_layer) {
        return planDD::build_bdd_by_layer(options.m_values);
    }

    if (options.m_values.build_sdd) {
        return planDD::build_sdd(options.m_values);
    }

    if (options.m_values.cnf_to_bdd) {
        return planDD::cnf_to_bdd(options.m_values);
    }

    if (options.m_values.encode_cnf) {
        return planDD::encode_cnf(options.m_values);
    }

    if (options.m_values.single_minisat) {
        return planDD::single_minisat(options.m_values);
    }

    if (options.m_values.count_minisat) {
        return planDD::count_minisat(options.m_values);
    }

    return -1;
}

int planDD::hack_debug(option_values opt_values) {
    LOG_MESSAGE(log_level::info) << "You unlocked full control. good luck modifying the source code";

    int num_variables = (opt_values.timesteps * 4) + 1;

    bdd_container copy_from(1, num_variables);
    bdd_container main_builder(1, num_variables);

    std::vector<int> exact_one;
    for (int i = 0; i < 4; i++) {
        exact_one.push_back(i + 1);
    }

    copy_from.add_exactly_one_constraint(exact_one);

    for (int t = 0; t < opt_values.timesteps; t++) {
        std::vector<int> indx_to;
        for (int i = 0; i < 4; i++) {
            indx_to.push_back((t * 4) + i + 1);
        }

        // copy_from.swap_variables(exact_one, indx_to);
        main_builder.copy_and_conjoin_bdd_from_another_container(copy_from);
        // copy_from.swap_variables(indx_to, exact_one);

        main_builder.write_bdd_to_dot_file("after_step_" + std::to_string(t) + ".dot");
        main_builder.print_bdd_info();
    }

    return 0;
}

int planDD::build_bdd(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    planning_logic::plan_to_cnf_map symbol_map;
    cnf_encoder encoder(opt_values, parser.m_sas_problem, symbol_map);
    encoder.initialize_symbol_map(opt_values.timesteps);

    std::vector<int> var_order = variable_order::order_variables(encoder, symbol_map, opt_values);
    bdd_container builder(1, symbol_map.get_num_variables());
    builder.set_variable_order(var_order);

    std::vector<planning_logic::logic_primitive> all_primitives = conjoin_order::order_all_clauses(encoder, opt_values);
    dd_builder::construct_dd_clause_linear(builder, all_primitives);
    builder.reduce_heap();

    builder.print_bdd_info();
    // builder.write_bdd_to_dot_file("normal_bdd.dot");
    return 0;
}

int planDD::build_bdd_by_layer(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    planning_logic::plan_to_cnf_map symbol_map;
    cnf_encoder encoder(opt_values, parser.m_sas_problem, symbol_map);
    encoder.initialize_symbol_map(opt_values.timesteps);

    std::vector<int> var_order = variable_order::order_variables(encoder, symbol_map, opt_values);
    bdd_container main_builder(opt_values.timesteps + 2, symbol_map.get_num_variables());
    // TODO think about variable order here
    // main_builder.set_variable_order(var_order);

    if (opt_values.bidirectional) {
        dd_builder::construct_bdd_by_layer_bidirectional(main_builder, encoder, symbol_map, opt_values);
    } else if (opt_values.exponential) {
        dd_builder::construct_dd_by_layer_exponentially(main_builder, encoder, symbol_map, opt_values);
    } else {
        dd_builder::construct_bdd_by_layer_unidirectional(main_builder, encoder, symbol_map, opt_values);
    }

    main_builder.print_bdd_info();

    return 0;
}

int planDD::build_sdd(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    planning_logic::plan_to_cnf_map symbol_map;
    cnf_encoder encoder(opt_values, parser.m_sas_problem, symbol_map);
    encoder.initialize_symbol_map(opt_values.timesteps);

    // encoder.write_cnf_to_file(opt_values.m_values.cnf_file, clauses);
    sdd_manager builder(symbol_map.get_num_variables());
    LOG_MESSAGE(log_level::info) << "Start building sdd";

    std::vector<planning_logic::logic_primitive> all_primitives = conjoin_order::order_all_clauses(encoder, opt_values);
    dd_builder::construct_dd_clause_linear(builder, all_primitives);
    builder.print_sdd();

    return 0;
}

// TODO fix this
int planDD::encode_cnf(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    planning_logic::plan_to_cnf_map symbol_map;
    cnf_encoder encoder(opt_values, parser.m_sas_problem, symbol_map);
    encoder.initialize_symbol_map(opt_values.timesteps);

    return 0;
}

int planDD::cnf_to_bdd(option_values opt_values) {
    std::tuple<int, int, std::vector<std::vector<int>>> cnf_data =
        planning_logic::formula::parse_cnf_file_to_clauses(opt_values.cnf_file);
    int num_variables = std::get<0>(cnf_data);
    // int num_clauses = std::get<1>(cnf_data);
    std::vector<std::vector<int>> clauses = std::get<2>(cnf_data);
    bdd_container builder(1, num_variables);

    for (std::vector<int> c : clauses) {
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
    builder.print_bdd_info();

    builder.write_bdd_to_dot_file("after_reorder.dot");

    return 0;
}

// TODO fix?
int planDD::single_minisat(option_values opt_values) {
    /*sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    cnf_encoder encoder(opt_values, parser.m_sas_problem);
    planning_logic::formula clauses = encoder.encode_cnf(opt_values.timesteps);
    clauses.write_to_file(opt_values.cnf_file);

    LOG_MESSAGE(log_level::info) << "Envoking minisat";
    std::string minisat_call = "../MiniSat_v1.14_linux " + opt_values.cnf_file + " " + opt_values.ass_file;
    std::system(minisat_call.c_str());

    std::vector<bool> assignment = encoder.parse_cnf_solution(opt_values.ass_file);
    encoder.decode_cnf_solution(assignment);
    */
    return 0;
}

// TODO fix?
int planDD::count_minisat(option_values opt_values) {
    // parse sas file
    /*
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    // encode it into cnf file
    cnf_encoder encoder(opt_values, parser.m_sas_problem);
    planning_logic::formula clauses = encoder.encode_cnf(opt_values.timesteps);
    clauses.write_to_file(opt_values.cnf_file);

    std::vector<bool> current_assignment;
    std::vector<bool> old_assignment;
    int num_solutions = 0;
    while (true) {
        // envoke minisat
        LOG_MESSAGE(log_level::debug) << "Envoking minisat, found " << num_solutions << " so far";
        std::string minisat_call = "../MiniSat_v1.14_linux " + opt_values.cnf_file + " " + opt_values.ass_file;
        std::system(minisat_call.c_str());
        current_assignment = encoder.parse_cnf_solution(opt_values.ass_file);

        // stop if no new solution was found
        if (current_assignment.size() == 0) break;

// print difference between both assignments
#if 0
            if(current_assignment.size() > 0 && old_assignment.size() > 0){
                encoder.compare_assignments(current_assignment, old_assignment);
            }
            old_assignment = current_assignment;
            encoder.decode_cnf_solution(current_assignment, opt_values.timesteps);
#endif

        // update the clauses with a new blockng one to prevent exactly the found solution
        std::vector<int> new_blocking_clause;
        for (int i = 1; i < current_assignment.size(); i++) {
            // value at index 0 has no meaning for sat solver (there is no variable 1)
            // add the negated variable (because of boolean algebra)
            new_blocking_clause.push_back(current_assignment[i] ? -i : i);
        }
        clauses.add_clause(new_blocking_clause, planning_logic::clause_none, -1);
        clauses.write_to_file(opt_values.cnf_file);
        num_solutions++;
    }

    LOG_MESSAGE(log_level::debug) << "Found a total of " << num_solutions << " solutions";
*/
    return 0;
}
