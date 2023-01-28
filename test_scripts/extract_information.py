import os
import re
import pickle
import matplotlib.pyplot as plt
import csv
import suites
import planDD_test_util_general as util



# extracts all the possible information from an output file into a dict
def compile_information_about_planDD_into_dic(domain_desc, file_path):
    info = {}
    # general information
    info["domain_desc"] = domain_desc
    info["file_path"] = file_path
    # information with multiple datapoints
    info["progress_to_time"] = extract_planDD_time_to_progress(file_path)
    return info

#symk also works for kstar
def compile_information_about_symk_into_dic(domain_desc, file_path):
    info = {}
    # general information
    info["domain_desc"] = domain_desc
    info["file_path"] = file_path
    # information with multiple datapoints
    info["progress_to_time"] = extract_symk_time_to_progress(file_path)
    return info

# returns a list of tuples that represents paths to output files from planDD
def find_all_output_files(suite_path):
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

# takes a suite_path and generated dictionaries that conatins information of the tescases
# these dicst get written to file for later reuse
def write_all_information_to_file(suite_path, output_path):
    all_files = find_all_output_files(suite_path)
    all_dics = []
    print(all_files)

    for i in range(len(all_files)):
        print("Compile information about testcase ", i+1, " from ", len(all_files))
        domain_desc, file_path = all_files[i]
        dic = {}
        if "planDD" in suite_path or "best_21_11" in suite_path:
            dic = compile_information_about_planDD_into_dic(domain_desc, file_path)
        elif "symk" in suite_path:
            dic = compile_information_about_symk_into_dic(domain_desc, file_path)
        elif "kstar" in suite_path:
            dic = compile_information_about_symk_into_dic(domain_desc, file_path)
        else:
            print("Error, unknown competitor")
        all_dics.append(dic)

    print("Dumping information to file")
    with open(output_path, "wb") as f:
        pickle.dump(all_dics, f)  
                
# Reads in the dicitonaries that were written to file eralier
def read_all_information_from_file(pickle_file):
    all_dics = []
    with open(pickle_file, 'rb') as f:
        all_dics = pickle.load(f)
    return all_dics

# time string is in format hh:mm:ss,sssss
def convert_time_string_to_float(time_string):
    return sum([a*b for a,b in zip([3600, 60, 1], map(float, time_string.split(":")))])


# returns a list of multiple data points during the execution of the program
def extract_planDD_time_to_progress(file_path):
    result = {}
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[(.*)\]\[info\] Found .* new plans in timestep .* new total is: (.*)", content)
        for x in lst:
            result[convert_time_string_to_float(x[0])] = float(x[1])
    return result

def extract_symk_time_to_progress(file_path):
    result = {}
    with open(file_path, "r") as f:
        content = str(f.read())
        lst1 = re.findall(r"\[t=(.*)s.*\] Attempted plans: (.*)M", content)
        lst2 = re.findall(r"\[t=(.*)s.*\] BOUND: [0-9]* < [0-9]* \[([0-9]*)\/[0-9]* plans\].*", content)
        for x in lst1:
            result[float(x[0])] = float(x[1])*(10**6)
        for x in lst2:
            result[float(x[0])] = float(x[1])
    return result

suite_names = [
    #"planDDTopK_k1000000000",
    #"planDDUseFD_k1000000000",
    #"planDDTopKRestart_k1000000000",
    #"planDDRestartUseFD_k1000000000",

    #"kstar_k1000000000",
    #"symk_k1000000000",

    #"planDD_31_11_k1000000000",
    #"planDDFixedRestart_k1000000000",
    #"planDDFixedRestartUseFD_k1000000000",

    # to check if restart is now was good as optimal
    #"planDDRestart_14_12_k1000000000",
    #"planDDIncremental_14_12_k1000000000",

    "T08_topK/T08_24_01_kstar_k1000000000",
    "T08_topK/T08_24_01_symk_k1000000000",
    #"T08_topK/T08_24_01_planDD_incremental_k1000000000",
    "T08_topK/T08_24_01_planDD_restart_k1000000000",
]

suite_dics = []

for x in suite_names:
    #write_all_information_to_file("../../final_results/" + x, "../../final_results/" + x + ".pkl")
    pass

for x in suite_names:
    suite_dics.append(read_all_information_from_file("../../final_results/" + x + ".pkl"))


# input: list of dictionaries and timebound
# output: list of (x,y) values with k on x axis and how many solved k on y axis
def get_k_to_solved_list(suite_dics, timebound):
    max_ks = []
    result = []
    for d in suite_dics:
        progress = d["progress_to_time"]
        values = [value for key, value in progress.items() if key <= timebound]
        if len(values) == 0:
            max_ks.append(0)
            #print("No solved on", d)
        else:
            max_ks.append(min(10**90, max(values)))
    max_ks.sort()

    num_suites = len(max_ks)
    print("Num suites", num_suites)
    for i in range(len(max_ks)):
        result.append((max_ks[i], num_suites-i))
    result.append((0, num_suites))
    result.sort(key = lambda x: (x[0],-x[1]))
    return result

#for i in range(len(suite_names)):
#    #print((suite_dics[i][0]))
#    plot_values = get_k_to_solved_list(suite_dics[i], 300)
#    xys = [(a,b) for a,b in plot_values if a >= 1]
#    #print(xys)
#    with open("../../final_results/" + suite_names[i] + "_kCoverage.csv", "w", newline='') as out_file:
#        writer = csv.writer(out_file)
#        writer.writerows(xys)
#    plt.plot([x for x,_ in plot_values], [y for _,y in plot_values], linestyle=":", marker="o", label=suite_names[i])

#plt.xscale("log")
#plt.legend()
#plt.show()

def list_coverage_on_domains(suitess, desired_k):
    all_problems = suites.suite_optimal_strips()
    unit_cost_problems = [
        p for p in all_problems if suites.TAG_HAS_ONLY_UNIT_COST_ACTIONS in suites.DOMAIN_TO_TAGS[p]
    ]

    suite_doms = {}
    for dom in unit_cost_problems:
        suite_doms[dom] = 0
        sanitized_dom = util.get_sanitized_domain_description(dom, "")
        for prob in suitess:
            #print(prob)
            if sanitized_dom in prob["domain_desc"]:
                for key, value in prob["progress_to_time"].items():
                    if value > desired_k:
                        suite_doms[dom] += 1
                        break
    return suite_doms


def write_dom_coverage():
    desired_ks = [10**3, 10**6, 10**9]
    num_ks = len(desired_ks)
    all_problems = suites.suite_optimal_strips()
    unit_cost_problems = [
        p for p in all_problems if suites.TAG_HAS_ONLY_UNIT_COST_ACTIONS in suites.DOMAIN_TO_TAGS[p]
    ]

    for dom in unit_cost_problems:
        with open("dom_coverages.tab", "a") as fil:
            fil.write("\\textsc{" + dom + "} & ")
            covs = {}
            for i in range(len(suite_dics)):
                suite = suite_dics[i]
                for des_k in desired_ks:
                    dom_coverage = list_coverage_on_domains(suite, des_k)
                    covs[(i , des_k)] = dom_coverage[dom]


            for i in range(len(suite_dics)):
                suite = suite_dics[i]
                for des_k in desired_ks:
                    c = covs[(i, des_k)]
                    is_biggest = True
                    for otherI in range(len(suite_dics)):
                        if otherI == i:
                            continue
                        otherC = covs[(otherI, des_k)]
                        if otherC > c:
                            is_biggest = False
                    if is_biggest and c > 0:
                        fil.write("\\textbf{" + str(c) + "} & ")
                    else:
                        fil.write(str(c) + " & ")
            fil.write("\\\\\n")


write_dom_coverage()
