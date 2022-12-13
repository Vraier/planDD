# the fast downard using topK planner is worse than the approach that finds all optimal approaches (even for k=1) this script tries to figure out, why

import os
import re
import suites

topk_path = "../test_output/best_30_11/planDDUseFD_k1000000000/"
optimal_path = "../test_output/best_21_11/best_21_11_old_best"

def get_all_testcases(suite_path):
    all_file_paths = []
    print(suite_path)
    for fold in os.listdir(suite_path):
        fold_path = os.path.join(suite_path, fold)
        if os.path.isdir(fold_path):
            for fil in os.listdir(fold_path):
                if fil == "output.txt" or fil == "planDD_output.txt":
                    file_path = os.path.join(fold_path, fil)
                    all_file_paths.append((fold, file_path))
                    break
    return all_file_paths

def unit_cost_domains():
    return [p for p in suites.suite_optimal_strips() if suites.TAG_HAS_ONLY_UNIT_COST_ACTIONS in suites.DOMAIN_TO_TAGS[p]]

def is_unit_cost_domain(d):
    for unit in unit_cost_domains():
        saintized_unit = "".join(x for x in unit if x.isalnum())
        if saintized_unit in d:
            return True
    return False

# time string is in format hh:mm:ss,sssss
def convert_time_string_to_float(time_string):
    return sum([a*b for a,b in zip([3600, 60, 1], map(float, time_string.split(":")))])

def extract_found_single_plan_for_topk(path):
    with open(path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[(.*)\]\[info\] Found (.*) new plans in timestep .* new total is: (.*)", content)
        for x in lst:
            if float(x[1]) > 0:
                return True
            #result[convert_time_string_to_float(x[0])] = float(x[2])
    return False

def extract_found_single_plan_optimal(path):
    with open(path, "r") as f:
        for line in f:
            p = re.compile("\[(.*)\]\[info\] Finished constructing final DD.*")
            if p.match(line):
                return True
    return False


def find_discrepanc_between_topk_optimal(path1, path2):
    cases1 = get_all_testcases(path1)
    cases2 = get_all_testcases(path2)
    #cases2 = [(d,p) for d,p in cases2 if is_unit_cost_domain(d)] # this probably fixed it, i also counted nonunitcost problems for optimal track
    # in addition i have to correctly create the variable for ever timestep, see problem psrsmallp42s82n3l4f50pddl to check if it got faster

    domains1 = [d for d,_ in cases1]
    domains2 = [d for d,_ in cases2]

    finished_domains1 = [d for d,p in cases1 if extract_found_single_plan_for_topk(p)]
    finished_domains2 = [d for d,p in cases2 if extract_found_single_plan_optimal(p)]

    print("Have a total of {} domains and finished {} for topk (suing fd)".format(len(domains1), len(finished_domains1)))
    print("Have a total of {} domains and finished {} for optimal".format(len(domains2), len(finished_domains2)))


    only_1 = [d for d in finished_domains1 if not d in finished_domains2]
    only_2 = [d for d in finished_domains2 if not d in finished_domains1]

    print("Only solved by topK")
    print(only_1)
    print("Only solved by optimal")
    print(only_2)


find_discrepanc_between_topk_optimal(topk_path, optimal_path)


