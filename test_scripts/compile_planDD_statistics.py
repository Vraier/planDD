from extract_planDD_information import *

import statistics

# print information if only a single run is of interest
def print_big_information_from_dicts(all_dics):
    non_unit_length_dics = [d for d in all_dics if d["error_while_encoding"]]
    not_finished_cnf_dics = [d for d in all_dics if not d["error_while_encoding"] and not d["has_finished_cnf"]]
    not_finished_dd_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished_cnf"] and not d["has_finished"]]
    solved_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished_cnf"] and d["has_finished"]]

    print("Compiling Information: ########################################################")
    print("Conjoin Order", get_conjoin_order_from_info(all_dics[0]))

    print("#Total", len(all_dics))
    print("#Non unit cost", len(non_unit_length_dics))
    print("#Not finished building CNF", len(not_finished_cnf_dics))
    print("#Not finished building DD", len(not_finished_dd_dics))
    print("#Solved", len(solved_dics))

    print("%Solved {:0.3f}".format(100*len(solved_dics)/(len(all_dics)-len(non_unit_length_dics))))

    average_time_spent_reordering_on_solved = statistics.mean([get_percentage_spent_reordering_from_info(i) for i in solved_dics])
    print("Average % of time spent reordering on solved test cases {:0.3f}".format(average_time_spent_reordering_on_solved))

    average_number_clauses_all = statistics.mean([i["constructed_clauses"] for i in solved_dics+not_finished_dd_dics])
    average_number_variables_all = statistics.mean([i["constructed_variables"] for i in solved_dics+not_finished_dd_dics])
    print("Average number of total clauses {:0.0f}, variables {:0.0f}".format(average_number_clauses_all, average_number_variables_all))
    average_number_clauses_solved = statistics.mean([i["constructed_clauses"] for i in solved_dics])
    average_number_variables_solved = statistics.mean([i["constructed_variables"] for i in solved_dics])
    print("Average number of total clauses/variables on solved test cases {:0.0f}/{:0.0f}".format(average_number_clauses_solved, average_number_variables_solved))
    average_percent_of_conjoined_clauses = statistics.mean([i["percent_conjoined_clauses"] for i in solved_dics+not_finished_dd_dics])
    print("Average % of conjoined clauses {:0.3f}".format(average_percent_of_conjoined_clauses))

    average_peak_of_nodes = statistics.mean([get_peak_number_of_nodes_from_info(i) for i in solved_dics])
    print("Average peak of nodes on solved test cases {:0.0f}".format(average_peak_of_nodes))
    average_peak_of_live_nodes = statistics.mean([get_peak_number_of_live_nodes_from_info(i) for i in solved_dics])
    print("Average peak of live nodes on solved test cases {:0.0f}".format(average_peak_of_live_nodes))
    average_bdd_nodes = statistics.mean([get_number_of_nodes_from_bdd_from_info(i) for i in solved_dics])
    print("Average number of nodes for bdd on solved test cases {:0.0f}".format(average_bdd_nodes))

# print information in multiple suites are of interest. 
# Have to modify the function i a way that ist also prints the difference between the suites (e.g. variable/conjoin order)
def get_small_information_from_dicts(all_dics):
    conjoin_order = get_conjoin_order_from_info(all_dics[0])
    solved_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished_cnf"] and d["has_finished"]]

    average_peak_of_nodes = statistics.mean([get_peak_number_of_nodes_from_info(i) for i in solved_dics])
    #print("Average peak of nodes on solved test cases {:0.0f}".format(average_peak_of_nodes))
    average_peak_of_live_nodes = statistics.mean([get_peak_number_of_live_nodes_from_info(i) for i in solved_dics])
    #print("Average peak of live nodes on solved test cases {:0.0f}".format(average_peak_of_live_nodes))
    average_bdd_nodes = statistics.mean([get_number_of_nodes_from_bdd_from_info(i) for i in solved_dics])
    #print("Average number of nodes for bdd on solved test cases {:0.0f}".format(average_bdd_nodes))

    info_string = "Conjoin Order: {}, #Solved: {}, Avg. peak nodes {:0.0f}, Avg. live nodes {:0.0f}, Avg. BDD size {:0.0f}".format(conjoin_order, len(solved_dics), average_peak_of_nodes, average_peak_of_live_nodes, average_bdd_nodes) 

    return (len(solved_dics), info_string)




#write_all_information_to_file("../test_output/small_easy_optimal_planDD_test", "../test_output/small_easy_optimal_planDD_test.pkl")
#write_all_information_to_file("../test_output/easy_optimal_planDD_test", "../test_output/easy_optimal_planDD_test.pkl")
#downward_write_all_information_to_file("easy_optimal_downward_test", "../test_output/easy_optimal_downward_test.pkl")

#small_planDD_dics = read_all_information_from_file("../test_output/small_easy_optimal_planDD_test.pkl")
#planDD_dics = read_all_information_from_file("../test_output/easy_optimal_planDD_test.pkl")
#print_information_from_dicts(small_planDD_dics)
#print_information_from_dicts(planDD_dics)
#downward_dics = read_all_information_from_file("../test_output/easy_optimal_downward_test.pkl")
#print(downward_dics)

suites = read_whole_set_of_tests_from_file("../test_output/big_interleaved")
compiled_infos = []

for s in suites:
    compiled_infos.append(get_small_information_from_dicts(s))

for (num_solved, info_text) in sorted(compiled_infos):
    print(info_text)

planDD_dics = read_all_information_from_file("../test_output/easy_optimal_planDD_test.pkl")
print_big_information_from_dicts(planDD_dics)