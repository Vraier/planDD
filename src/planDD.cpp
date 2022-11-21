#include "planDD.h"

#include <stdio.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "bdd_container.h"
#include "dd_builder.h"
#include "dd_builder_conjoin_order.h"
#include "dd_builder_topk.h"
#include "dd_builder_variable_order.h"
#include "encoder_basic.h"
#include "encoder_binary_parallel.h"
#include "graph.h"
#include "logging.h"
#include "order_visualization.h"
#include "plan_to_cnf_map.h"
#include "sas_parser.h"
#include "sdd_container.h"
#include "variable_creation.h"

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
    if (options.m_values.conflict_graph) {
        return planDD::conflict_graph(options.m_values);
    }

    if (options.m_values.build_bdd) {
        return planDD::build_bdd(options.m_values);
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

    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    encoder::encoder_basic encoder(opt_values, parser.m_sas_problem);

    bdd_container builder(2);

    if (opt_values.no_reordering) {
        builder.disable_reordering();
    } else {
        builder.enable_reordering();
    }

    dd_builder::construct_dd_top_k(builder, encoder, opt_values);

    builder.print_info();

    return 0;

    /*
    LOG_MESSAGE(log_level::info) << "Outputting order";
    for(int i = 0; i < var_order.size(); i ++){
        planning_logic::tagged_variable tv = encoder->m_symbol_map.get_planning_info_for_variable(var_order[i]);
        std::cout << "tag=" << std::get<0>(tv) << " ";
        std::cout << "t=" << std::get<1>(tv) << " ";
        if(std::get<0>(tv) == planning_logic::variable_plan_var){
            std::cout << parser.m_sas_problem.m_variabels[std::get<2>(tv)].to_string() << " ";
        } else if(std::get<0>(tv) == planning_logic::variable_plan_op){
            std::cout << parser.m_sas_problem.m_operators[std::get<2>(tv)].to_string() << " ";
        }
        std::cout << "v=" << std::get<3>(tv) << std::endl;
    }*/

    // for(auto a: builder.list_minterms(10)){
    // encoder.decode_cnf_solution(a, 5);
    //}
    // builder.write_bdd_to_dot_file("normal_bdd.dot");
}

int planDD::conflict_graph(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    LOG_MESSAGE(log_level::info) << "Starting to construct conflict graph";

    graph::undirected_graph complement_graph = parser.m_sas_problem.construct_complement_action_conflic_graph();
    std::vector<int> colouring = graph::approximate_colouring(complement_graph);
    int max = 0;
    for (int i = 0; i < colouring.size(); i++) {
        max = colouring[i] > max ? colouring[i] : max;
    }

    LOG_MESSAGE(log_level::info) << "Num colours: " << max + 1;

    graph::write_to_file_with_colouring("graph.dot", complement_graph, colouring, false);

    LOG_MESSAGE(log_level::info) << "Finished writing graph";

    return 0;
}

int planDD::build_bdd(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    encoder::encoder_abstract *encoder;
    if (opt_values.binary_parallel) {
        graph::undirected_graph conflict_graph = parser.m_sas_problem.construct_complement_action_conflic_graph();
        encoder = new encoder::binary_parallel(opt_values, parser.m_sas_problem, conflict_graph);
    } else {
        encoder = new encoder::encoder_basic(opt_values, parser.m_sas_problem);
    }

    bdd_container builder(1);

    if (opt_values.timesteps >= 0) {
        variable_creation::create_variables_for_first_t_steps(opt_values.timesteps, *encoder, builder, opt_values);
        std::vector<int> var_order = variable_order::order_variables(*encoder, opt_values);

        // std::cout << "var order: ";
        // for(int a: var_order){
        //     std::cout << a << " ";
        // }
        // std::cout << std::endl;
        builder.set_variable_order(var_order);
    }

    if (opt_values.no_reordering) {
        builder.disable_reordering();
    } else {
        builder.enable_reordering();
    }

    dd_builder::construct_dd(builder, *encoder, opt_values);

    builder.print_info();

    delete encoder;

    // for(auto a: builder.list_minterms(10)){
    // encoder.decode_cnf_solution(a, 5);
    //}
    // builder.write_bdd_to_dot_file("normal_bdd.dot");
    return 0;
}

int planDD::build_sdd(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    encoder::encoder_basic encoder(opt_values, parser.m_sas_problem);
    sdd_container container(1, encoder.num_variables_in_t_timesteps(opt_values.timesteps));
    variable_creation::create_variables_for_first_t_steps(opt_values.timesteps, encoder, container, opt_values);
    std::vector<planning_logic::logic_primitive> all_primitives = conjoin_order::order_all_clauses(encoder, opt_values);

    LOG_MESSAGE(log_level::info) << "Start building sdd";

    dd_builder::conjoin_primitives_linear(container, all_primitives);
    container.print_info();

    return 0;
}

// TODO fix this
int planDD::encode_cnf(option_values opt_values) {
    sas_parser parser(opt_values.sas_file);
    if (parser.start_parsing() == -1) {
        LOG_MESSAGE(log_level::error) << "Error while parsing sas_file";
        return 0;
    }

    // cnf_encoder encoder(opt_values, parser.m_sas_problem);
    // encoder.initialize_symbol_map(opt_values.timesteps);

    return 0;
}

int planDD::cnf_to_bdd(option_values opt_values) {
    std::tuple<int, int, std::vector<std::vector<int>>> cnf_data =
        planning_logic::formula::parse_cnf_file_to_clauses(opt_values.cnf_file);
    int num_variables = std::get<0>(cnf_data);
    // int num_clauses = std::get<1>(cnf_data);
    std::vector<std::vector<int>> clauses = std::get<2>(cnf_data);
    bdd_container builder(1);

    for (std::vector<int> c : clauses) {
        builder.add_clause_primitive(c);
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
    builder.print_info();

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
