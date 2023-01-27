from xml.dom.expatbuilder import theDOMImplementation
from extract_planDD_information import *
import matplotlib.pyplot as plt
import numpy as np
import csv


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
    print("Var Order", all_dics[0]["config_variable_order"])

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
def print_solving_difference_between_two_configs(dics1, name1, dics2, name2):
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


def plot_suites(suite_names, suite_dics):
    suite_points = []
    for i in range(len(suite_dics)):
        dics = suite_dics[i]
        times = []
        for d in dics:
            if(d["has_finished"]):
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

    for i in range(len(suite_dics)):
        ps = suite_points[i]
        plt.plot([x for x,_ in ps], [y for _,y in ps], linestyle=":", marker="o", label=suite_names[i])
    plt.legend()
    plt.show()

def count_num_finished(suites):
    finished = [x for x in suites if x["has_finished"] == True]
    return len(finished)

def get_finish_time_num_finished_tuples(suites):
    finish_times = []
    for x in suites:
        if x["has_finished"] == True:
            finish_times.append(x["finish_time"])
    finish_times.sort()
    res = []
    for i in range(len(finish_times)):
        res.append((finish_times[i], i+1))
    return res

# writing tests to pickle files
#write_whole_set_of_tests_to_file("../../final_results/T01_clauseOrder")

variable_order_differences = [
        "config_build_order", 
]

# results for multiple dicts
#all_dics = read_whole_set_of_tests_from_file("../../final_results/T01_clauseOrder")
#print("Done reading all dicts")

#print_information_for_multiple_dicts(all_dics, variable_order_differences)

#all_num_finished = []
#for suites in all_dics:
#    all_num_finished.append(count_num_finished(suites))

#plt.hist(all_num_finished, density=False, bins=70)
#plt.show()

#hist_xs = list(range(76))
#hist_ys = [0]*75
#for x in all_num_finished:
#    hist_ys[x] += 1

#hist_xys = list(zip(hist_xs, hist_ys))
#with open("../../final_results/T01_clauseOrder/hist.csv", "w", newline='') as out_file:
#        writer = csv.writer(out_file)
#        writer.writerows(hist_xys)

#with open("../../final_results/T01_clauseOrder/hist_pure.csv", "w") as out_file:
#    for x in all_num_finished:
#        out_file.write(str(x) + "\n")
    

suite_names = [
    #"T02_varOrder/T02_11_01_ord_0_k1000000000",
    #"T02_varOrder/T02_11_01_ord_0_no_reorder_k1000000000",
    #"T02_varOrder/T02_11_01_ord_1_k1000000000",
    #"T02_varOrder/T02_11_01_ord_1_no_reorder_k1000000000",
    #"T02_varOrder/T02_11_01_ord_2_k1000000000",
    #"T02_varOrder/T02_11_01_ord_2_no_reorder_k1000000000",
    #"T02_varOrder/T02_11_01_ord_3_k1000000000",
    #"T02_varOrder/T02_11_01_ord_3_no_reorder_k1000000000",

    "T04_naiv/T04_18_01_naiv_bdd_k1000000000",
    "T04_naiv/T04_18_01_naiv_sdd_k1000000000",

    #"T03_construction/T03_15_01_bi_k1000000000",
    #"T03_construction/T03_15_01_bi_share_k1000000000",
    #"T03_construction/T03_15_01_bi_share_perm_k1000000000",
    #"T03_construction/T03_15_01_expo_k1000000000",
    #"T03_construction/T03_15_01_layer_k1000000000",
    #"T03_construction/T03_15_01_layer_perm_k1000000000",

    #"T06_encoding/T06_18_01_encoding_binary_op_k1000000000",
    #"T06_encoding/T06_18_01_encoding_binary_var_k1000000000",
    #"T06_encoding/T06_18_01_encoding_binary_var_op_k1000000000",
    #"T06_encoding/T06_18_01_encoding_binary_var_op_exclude_k1000000000",
    #"T06_encoding/T06_18_01_encoding_direct_k1000000000",
    #"T06_encoding/T06_18_01_encoding_ladder_k1000000000",
    #"T06_encoding/T06_18_01_encoding_naiv_k1000000000",
    #"T06_encoding/T06_18_01_encoding_parallel_k1000000000",
    #"T06_encoding/T06_18_01_encoding_parallel_binary_k1000000000",

    #"T05_query/T05_18_01_common_operator_k1000000000",
    #"T05_query/T05_18_01_random_plans_k1000000000",
]
all_dics = []

#write_all_information_to_file("../../final_results/T04_naiv/T04_18_01_naiv_bdd_k1000000000", "../../final_results/T04_naiv/T04_18_01_naiv_bdd_k1000000000.pkl")
#write_sdd_compiler_to_file("../../final_results/T04_naiv/T04_18_01_naiv_sdd_k1000000000", "../../final_results/T04_naiv/T04_18_01_naiv_sdd_k1000000000.pkl")

for x in suite_names:
    #write_all_information_to_file("../../final_results/" + x, "../../final_results/" + x + ".pkl")
    pass

for x in suite_names:
    all_dics.append(read_all_information_from_file("../../final_results/" + x + ".pkl"))

for i in range(len(suite_names)):
    suites = all_dics[i]
    xys = get_finish_time_num_finished_tuples(suites)
    print(xys)
    with open("../../final_results/" + suite_names[i] + ".csv", "w", newline='') as out_file:
        writer = csv.writer(out_file)
        writer.writerows(xys)


def query_analysis():
    random_plan_dics = read_all_information_from_file("../../final_results/T05_query/T05_18_01_random_plans_k1000000000.pkl")
    common_op_dics = read_all_information_from_file("../../final_results/T05_query/T05_18_01_common_operator_k1000000000.pkl")

    num_solutions = []
    started_counting = 0
    finished_counting = 0
    counting_times = []
    relative_counting_times = []
    for d in common_op_dics:
        if d["num_solutions"] >= 0:
            num_solutions.append(d["num_solutions"])
        if d["query_common_start"] >= 0:
            started_counting += 1
        if d["query_common_end"] >= 0:
            finished_counting += 1
            counting_time = d["query_common_end"]-d["query_common_start"]
            relative_counting_time = counting_time/d["query_common_end"]
            counting_times.append(counting_time)
            relative_counting_times.append(relative_counting_time)

    started_sampling = 0
    finished_sampling = 0
    sampling_times = []
    relative_sampling_times = []
    for d in random_plan_dics:
        if d["query_random_start"] >= 0:
            started_sampling += 1
        if d["query_random_end"] >= 0:
            finished_sampling += 1
            sampling_time = d["query_random_end"]-d["query_random_start"]
            relative_sampling_time = counting_time/d["query_random_end"]
            sampling_times.append(sampling_time)
            relative_sampling_times.append(relative_sampling_time)

    print("Number of solutions", sorted(num_solutions))
    print("Started counting", started_counting, "Finished counting", finished_counting)
    print("Counting times", sorted(counting_times))
    print("Relative counting times", sorted(relative_counting_times))

    print("Started sampling", started_sampling, "Finished sampling", finished_sampling)
    print("Sampling times", sorted(sampling_times))
    print("Relative sampling times", sorted(relative_sampling_times))

# compares two configurations by building a scatter plot
def scatter_plot_compare_two_configs(info_dics_config1, info_dics_config2, timeout, name1, name2, csv_name):
    points = []

    dom_to_dic1 = {}
    for prob_dic in info_dics_config1:
        dom_to_dic1[prob_dic["domain_desc"]] = prob_dic

    add_info = []
    for prob_dic2 in info_dics_config2:
        if not prob_dic2["domain_desc"] in dom_to_dic1:
            print("Warning")
            continue
        prob_dic1 = dom_to_dic1[prob_dic2["domain_desc"]]

        time1 = prob_dic1["finish_time"] if prob_dic1["finish_time"] > 0 and prob_dic1["finish_time"] <= timeout else timeout * 1.2
        time2 = prob_dic2["finish_time"] if prob_dic2["finish_time"] > 0 and prob_dic1["finish_time"] <= timeout else timeout * 1.2

        if(time1 <= 300 or time2 <= 300):
            add_info.append((prob_dic2["domain_desc"], time1, time2))
        points.append((time1, time2))

    for x in sorted(add_info):
        print(x)

    fig, ax = plt.subplots()
    ax.set_aspect('equal', adjustable='box')
    ax.scatter([x for x, y in points], [y for x, y in points])
    ax.plot([0,timeout],[0,timeout], 'red', linewidth=1)
    plt.xlabel(name1)
    plt.ylabel(name2)
    plt.show()
    with open("../../final_results/" + csv_name, "w", newline='') as out_file:
        writer = csv.writer(out_file)
        writer.writerows(points)


for i in range(len(suite_names)):
    print(suite_names[i])
    #print_big_information_from_dicts(all_dics[i])
    pass

plot_suites(suite_names, all_dics)


#for suites in all_dics:
#    xys = get_finish_time_num_finished_tuples(suites)
#    xs = [x for x,_ in xys]
#    ys = [y for _,y in xys]

#    plt.plot(xs,ys, color="blue")

#plt.show()

# for query analysis
#query_analysis()

# for naiv comparison
naive_bdd = read_all_information_from_file("../../final_results/T04_naiv/T04_18_01_naiv_bdd_k1000000000.pkl")
naive_sdd = read_all_information_from_file("../../final_results/T04_naiv/T04_18_01_naiv_sdd_k1000000000.pkl")
print(naive_sdd[0])
scatter_plot_compare_two_configs(naive_bdd, naive_sdd, 300, "bdd", "sdd", "naive_scatter.csv")