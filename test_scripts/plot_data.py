from matplotlib import pyplot as plt
from os.path import isfile, join
from functools import cmp_to_key

import os
import re

TIMEOUT = 600
NUM_CLAUSES = 9396 


# time string is in format hh:mm:ss,sssss
def convert_time_string_to_float(time_string):
    return sum([a*b for a,b in zip([3600, 60, 1], map(float, time_string.split(":")))])

# returns a list of times. At each timepoint a ne conjon operation was finished
def extract_conjoin_progress(file_path):
    conjoin_times = []
    with open(file_path, "r") as f:
        lines = f.readlines()
        for l in lines:
            p = re.compile("\[(.*)\]\[info\].* Conjoined (.*) clauses")
            if p.match(l):
                time_string = p.search(l).group(1)
                time = convert_time_string_to_float(time_string)
                conjoin_times.append(time)
    return conjoin_times

# returns wether the test_run finished building the DD
def extract_has_finished(file_path):
    with open(file_path, "r") as f:
        lines = f.readlines()
        for l in lines:
            p = re.compile("\[.*\]\[info\].* Finished constructing DD.*")
            if p.match(l):
                return True
    return False

# returns the time at which the DD was build
def extract_finish_time(file_path):
    with open(file_path, "r") as f:
        lines = f.readlines()
        for l in lines:
            p = re.compile("\[(.*)\]\[info\].* Finished constructing DD.*")
            if p.match(l):
                time_string = p.search(l).group(1)
                return convert_time_string_to_float(time_string)
    return TIMEOUT

# returns the conjoin order that was used during construction
def extract_order(file_path):
    with open(file_path, "r") as f:
        lines = f.readlines()
        for l in lines:
            p = re.compile("\[.*\]\[info\].* Construction DD with the following .*order: (.*)")
            if p.match(l):
                order_string = p.search(l).group(1)
                return order_string
    print("[Warning] Found no order")
    return ""

# lists all "test_run" files in a folder
def get_all_test_files(folder_path):
    return [join(folder_path, f) for f in os.listdir(folder_path) if isfile(join(folder_path, f)) and "test_run" in f]


def plot_conjoin_times(folder_path):
    all_files = sorted(get_all_test_files(folder_path))

    for fil in all_files:
        times = extract_conjoin_progress(fil)
        y_axis = list(range(1, len(times)+1))

        if not extract_has_finished(fil):
            times.append(180)
            y_axis.append(y_axis[-1])
        plt.plot(times, y_axis)
    
    plt.show()


def custom_triple_sort(a, b):
    if a[1] > b[1]: return 1
    elif b[1] > a[1]: return -1
    else:
        if a[2] > b[2]: return -1
        elif b[2] > a[2]: return 1
        else: return 0

def simplify_order(order_string):
    new_string = order_string.replace("rtyum", "m", 1)
    new_string = new_string.replace("pe", "p", 1)
    # special case to fix earlier order representation
    if not ":" in new_string:
        new_string += "x:"
    return new_string

def unsimplify_order_string(order_string):
    new_string = order_string.replace("m", "rtyum", 1)
    new_string = new_string.replace("p", "pe", 1)
    return new_string


# returns a dict of triples (on for every test) 
# mapping order to tuple with, first is time, second is #conjoined clauses
# the triples are sorted by filenames
def get_information_about_order_performance(folder_path):
    all_files = sorted(get_all_test_files(folder_path))

    order_statistics = {}

    for fil in all_files:
        order = simplify_order(extract_order(fil))
        finish_time = extract_finish_time(fil)
        total_conjoins = len(extract_conjoin_progress(fil))

        order_statistics[order] = (finish_time, total_conjoins)

    return order_statistics

# desinged to compared bdd and sdd run
def print_comparison_of_two_order_runs(folder1, folder2):
    info1 = get_information_about_order_performance(folder1)
    info2 = get_information_about_order_performance(folder2)

    # print(info2)

    joind_info = {order : info1[order] + info2[order] for order in info1 if order in info2}
    list_info = [(order, joind_info[order][0], joind_info[order][1], joind_info[order][2], joind_info[order][3]) for order in joind_info]


    list_info.sort(key=cmp_to_key(custom_triple_sort))

    for x in list_info:
        print(x)

# simple way to output the performance of different orders in a folder
def print_all_orders_sorted(folder_path):
    order_dict = get_information_about_order_performance(folder_path)
    order_list = [(order, order_dict[order][0], order_dict[order][1]) for order in order_dict]

    order_list.sort(key=cmp_to_key(custom_triple_sort))

    for x in order_list:
        print(x)


def find_all_finished_orders(folder_path):
    order_dict = get_information_about_order_performance(folder_path)
    finished = [order for order in order_dict if order_dict[order][0] != TIMEOUT]
    return finished



#plot_conjoin_times("../test_output/simple_orders_bdd")
#plot_conjoin_times("../test_output/simple_orders_sdd")
#plot_conjoin_times("../test_output/interleaved_bdd")
#plot_conjoin_times("../test_output/include_mutex")
##plot_conjoin_times("../test_output/reverse_order")

#print_all_orders_sorted("../test_output/simple_orders_bdd")
#print_all_orders_sorted("../test_output/simple_orders_sdd")
print_all_orders_sorted("../test_output/interleaved_bdd")
#print(find_all_finished_orders("../test_output/interleaved_bdd"))

#print(get_information_about_order_performance("../test_output/simple_orders_sdd"))

#print_comparison_of_two_order_runs("../test_output/simple_orders_bdd", "../test_output/simple_orders_sdd")
#print_comparison_of_two_order_runs("../test_output/interleaved_bdd", "../test_output/include_mutex")
##print_comparison_of_two_order_runs("../test_output/interleaved_bdd", "../test_output/reverse_order")
