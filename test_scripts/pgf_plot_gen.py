from xml.dom.expatbuilder import theDOMImplementation
from extract_planDD_information import *
import matplotlib.pyplot as plt

import statistics

# print information if only a single run is of interest
def print_big_information_from_dicts(all_dics):
    error_while_encoding = [d for d in all_dics if d["error_while_encoding"]]
    not_finished_cnf_dics = [d for d in all_dics if not d["error_while_encoding"] and not d["has_finished_cnf"]]
    not_finished_dd_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished_cnf"] and not d["has_finished"]]
    solved_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished"]]

    time_for_solved = [(d["finish_time"], d["domain_desc"]) for d in solved_dics]
    time_for_solved = sorted(time_for_solved)
    #print("Compiling Solved Testcases: ############################################################################################")
    print(sorted(time_for_solved, key = lambda x: x[1]))

    print("Compiling Information: #################################################################################################")
    print("Conjoin Order", all_dics[0]["config_build_order"])

    print("#Total", len(all_dics))
    print("#Error encoding", len(error_while_encoding))
    print("#Not finished building CNF", len(not_finished_cnf_dics))
    print("#Not finished building DD", len(not_finished_dd_dics))
    print("#Solved", len(solved_dics))

    print("%Solved {:0.3f}".format(100*len(solved_dics)/(len(all_dics)-len(error_while_encoding))))

    average_time_spent_reordering_on_solved = statistics.mean([get_percentage_spent_reordering_from_info(i) for i in solved_dics])
    print("Average % of time spent reordering on solved test cases {:0.3f}".format(average_time_spent_reordering_on_solved))

    average_number_clauses_all = statistics.mean([i["constructed_clauses"] for i in solved_dics+not_finished_dd_dics])
    average_number_variables_all = statistics.mean([i["constructed_variables"] for i in solved_dics+not_finished_dd_dics])
    print("Average number of total clauses/variables {:0.0f}/{:0.0f}".format(average_number_clauses_all, average_number_variables_all))
    average_number_clauses_solved = statistics.mean([i["constructed_clauses"] for i in solved_dics])
    average_number_variables_solved = statistics.mean([i["constructed_variables"] for i in solved_dics])
    print("Average number of total clauses/variables on solved test cases {:0.0f}/{:0.0f}".format(average_number_clauses_solved, average_number_variables_solved))
    average_percent_of_conjoined_clauses = statistics.mean([i["last_conjoin_percent"] if i["last_conjoin_percent"] >= 0 else 0 for i in solved_dics+not_finished_dd_dics])
    print("Average % of conjoined clauses {:0.3f}".format(average_percent_of_conjoined_clauses))

    average_peak_of_nodes = statistics.mean([get_peak_number_of_nodes_from_info(i) for i in solved_dics])
    print("Average peak of nodes on solved test cases {:0.0f}".format(average_peak_of_nodes))
    average_peak_of_live_nodes = statistics.mean([get_peak_number_of_live_nodes_from_info(i) for i in solved_dics])
    print("Average peak of live nodes on solved test cases {:0.0f}".format(average_peak_of_live_nodes))
    average_bdd_nodes = statistics.mean([get_number_of_nodes_from_bdd_from_info(i) for i in solved_dics])
    print("Average number of nodes for bdd on solved test cases {:0.0f}".format(average_bdd_nodes))

    average_colouring_time = statistics.mean([i["finish_colouring"] for i in solved_dics])
    average_colours = statistics.mean([i["num_colour_classes"] for i in solved_dics])
    print("Average number of colouring time and colours: {:0.3f}, {:0.1f}".format(average_colouring_time, average_colours))

# print information in multiple suites are of interest.
# key arguments hold the config values that differentiate between the runs
def get_small_information_from_dicts(all_dics, key_arguments):
    config_discription = ""
    for a in key_arguments:
        config_discription += a[7:] + ": " + all_dics[0][a] + " "

    example_filepath = all_dics[0]["file_path"]
    solved_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished"]]

    average_peak_of_nodes = statistics.mean([get_peak_number_of_nodes_from_info(i) for i in solved_dics])
    average_peak_of_live_nodes = statistics.mean([get_peak_number_of_live_nodes_from_info(i) for i in solved_dics])
    average_bdd_nodes = statistics.mean([get_number_of_nodes_from_bdd_from_info(i) for i in solved_dics])
    average_finish_time = statistics.mean([i["finish_time"] for i in solved_dics])

    info_string = "{} #Solved: {}, Avg. finish time {:0.2f}s, Avg. peak nodes {:0.0f}, Avg. live nodes {:0.0f}, Avg. BDD size {:0.0f}".format(config_discription, len(solved_dics), average_finish_time, average_peak_of_nodes, average_peak_of_live_nodes, average_bdd_nodes) 

    return (len(solved_dics), example_filepath, info_string)

def print_information_for_multiple_dicts(suites, config_differences):
    compiled_infos = []
    for s in suites:
        compiled_infos.append(get_small_information_from_dicts(s, config_differences))
    
    sorted_infos = sorted(compiled_infos)
    for (num_solved, example_filepath, info_text) in sorted_infos:
        print(info_text)

    num_best_candidates = min(10, len(sorted_infos))
    print("Printing", num_best_candidates, "best candidates")
    for i in range(num_best_candidates):
        print(str(i+1), sorted_infos[::-1][i][1])
     
# prints which problems where only solved by the one config and not the other   
def print_solving_difference_between_two_congigs(dics1, name1, dics2, name2):
    solved1 = set([d["domain_desc"] for d in dics1 if not d["error_while_encoding"] and d["has_finished"]])
    solved2 = set([d["domain_desc"] for d in dics2 if not d["error_while_encoding"] and d["has_finished"]])
    print("Only solved by", name1, ": ")
    print(solved1 - solved2)
    print("Only solved by", name2, ": ")
    print(solved2 - solved1)

def print_portfolio_information(suites):
    curr_solved = set()
    prev_solved = set()

    for suite in suites:
        solved_dics = [d for d in suite if not d["error_while_encoding"] and d["has_finished"]]
        for d in solved_dics:
            curr_solved.add(d["domain_desc"])
        print("##################################################################################")
        print("Solved a total of", len(curr_solved), "problems")
        print("Solved", len(curr_solved) - len(prev_solved), "more problems than before")
        prev_solved = set(curr_solved)

    print(curr_solved)


def plot_suites(suite_dics):
    suite_points = []
    for i in range(len(suite_dics)):
        dics = suite_dics[i]
        times = []
        for d in dics:
            t = d["finish_time"]
            if t >= 0 and t <= 600:
                times.append(t)
        times.sort()
        points = []
        total = 1
        for t in times:
            points.append((t, total))
            total += 1
        suite_points.append(points)

    for ps in suite_points:
        plt.plot([x for x,_ in ps], [y for _,y in ps], linestyle=":", marker="o", label=suite_names[i])
    plt.show()

# writing tests to pickle files
#write_whole_set_of_tests_to_file("../../test_output/T01_clauseOrder")

variable_order_differences = [
        "config_build_order", 
]

# results for multiple dicts
print_information_for_multiple_dicts(read_whole_set_of_tests_from_file("../../test_output/T01_clauseOrder"), variable_order_differences)
#print_ifnormation_for_multiple_dicts(read_whole_set_of_tests_from_file("../test_output/variable_order_no_ladder"))
#print_ifnormation_for_multiple_dicts(read_whole_set_of_tests_from_file("../test_output/variable_order_with_ladder"))
