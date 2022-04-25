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

def extract_has_finished(file_path):
    with open(file_path, "r") as f:
        lines = f.readlines()
        for l in lines:
            p = re.compile("\[.*\]\[info\].* Finished constructing DD.*")
            if p.match(l):
                return True
    return False

def extract_finish_time(file_path):
    with open(file_path, "r") as f:
        lines = f.readlines()
        for l in lines:
            p = re.compile("\[(.*)\]\[info\].* Finished constructing DD.*")
            if p.match(l):
                time_string = p.search(l).group(1)
                return convert_time_string_to_float(time_string)
    return TIMEOUT

def extract_order(file_path):
    with open(file_path, "r") as f:
        lines = f.readlines()
        for l in lines:
            p = re.compile("\[.*\]\[info\].* Construction DD with the following order: (.*)")
            if p.match(l):
                order_string = p.search(l).group(1)
                return order_string
    print("[Warning] Found no order")
    return ""

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
    return new_string


def print_all_orders_sorted(folder_path):
    all_files = sorted(get_all_test_files(folder_path))

    order_statistics = []

    for fil in all_files:
        order = simplify_order(extract_order(fil))
        finish_time = extract_finish_time(fil)
        total_conjoins = len(extract_conjoin_progress(fil))

        order_statistics.append((order, finish_time, total_conjoins))
        
    order_statistics.sort(key=cmp_to_key(custom_triple_sort))

    for x in order_statistics:
        print(x)



plot_conjoin_times("all_order_permutations")

#plot_conjoin_times("sdd_try")


#print_all_orders_sorted("all_order_permutations")