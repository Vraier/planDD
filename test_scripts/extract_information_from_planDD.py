import re
import os
import pickle
import statistics

# returns al list of tuple that represents paths to output files from planDD
def find_all_output_files(suite_path):
    all_file_paths = []
    for fold in os.listdir(suite_path):
        fold_path = os.path.join(suite_path, fold)
        if os.path.isdir(fold_path):
            for fil in os.listdir(fold_path):
                if "planDD_output" in fil:
                    file_path = os.path.join(fold_path, fil)
                    all_file_paths.append((fold, file_path))
                    break
    return all_file_paths

def compile_information_about_planDD(domain_desc, file_path):
    info = {}
    info["domain_desc"] = domain_desc
    info["has_finished_cnf"] = extract_has_finished_constructing_cnf(file_path)
    info["has_finished"] = extract_has_finished(file_path)
    info["error_while_encoding"] = extract_has_error_while_encoding_to_sat(file_path)
    info["finish_time"] = extract_finish_time(file_path)
    info["config"] = extract_config(file_path)
    info["cudd_config"] = extract_CUDD_config(file_path)
    info["constructed_clauses"] = extract_total_constructed_clauses(file_path)
    info["conjoined_clauses"] = extract_conjoined_clauses(file_path)

    return info

def write_all_information_to_file(suite_path, output_path):
    all_files = find_all_output_files(suite_path)
    all_dics = []

    for i in range(len(all_files)):
        print("Compile information about testcase ", i+1, " from ", len(all_files))
        domain_desc, file_path = all_files[i]
        dic = compile_information_about_planDD(domain_desc, file_path)
        all_dics.append(dic)

    print("Dumping information to file")
    with open(output_path, "wb") as f:
        pickle.dump(all_dics, f)

def read_all_information_from_file(pickle_file):
    all_dics = []
    with open(pickle_file, 'rb') as f:
        all_dics = pickle.load(f)
    return all_dics

def print_information_from_dicts(all_dics):
    non_unit_length_dics = [d for d in all_dics if d["error_while_encoding"]]
    not_finished_cnf_dics = [d for d in all_dics if not d["error_while_encoding"] and not d["has_finished_cnf"]]
    not_finished_dd_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished_cnf"] and not d["has_finished"]]
    solved_dics = [d for d in all_dics if not d["error_while_encoding"] and d["has_finished_cnf"] and d["has_finished"]]

    print(sorted([i["domain_desc"] for i in solved_dics]))

    print("#Total", len(all_dics))
    print("#Non unit cost", len(non_unit_length_dics))
    print("#Not finished building CNF", len(not_finished_cnf_dics))
    print("#Not finished building DD", len(not_finished_dd_dics))
    print("#Solved", len(solved_dics))

    print("%Solved {:0.3f}".format(100*len(solved_dics)/(len(all_dics)-len(non_unit_length_dics))))

    average_time_spent_reordering_on_solved = statistics.mean([get_percentage_spent_reordering_from_info(i) for i in solved_dics])
    print("Average % of time spent reordering on solved test cases {:0.3f}".format(average_time_spent_reordering_on_solved))

    average_number_clauses_all = statistics.mean([i["constructed_clauses"] for i in solved_dics+not_finished_dd_dics])
    print("Average number of total clauses {:0.0f}".format(average_number_clauses_all))
    average_number_clauses_solved = statistics.mean([i["constructed_clauses"] for i in solved_dics])
    print("Average number of total clauses on solved test cases {:0.0f}".format(average_number_clauses_solved))
    average_percent_of_conjoined_clauses = statistics.mean([get_percentage_of_conjoined_clauses_from_info(i) for i in solved_dics+not_finished_dd_dics])
    print("Average % of conjoined clauses {:0.3f}".format(average_percent_of_conjoined_clauses))

    average_peak_of_nodes = statistics.mean([get_peak_number_of_nodes_from_info(i) for i in solved_dics])
    print("Average peak of nodes on solved test cases {:0.0f}".format(average_peak_of_nodes))
    average_peak_of_live_nodes = statistics.mean([get_peak_number_of_live_nodes_from_info(i) for i in solved_dics])
    print("Average peak of nodes live on solved test cases {:0.0f}".format(average_peak_of_live_nodes))
    average_bdd_nodes = statistics.mean([get_number_of_nodes_from_bdd_from_info(i) for i in solved_dics])
    print("Average number of nodes for bdd on solved test cases {:0.0f}".format(average_bdd_nodes))

# time string is in format hh:mm:ss,sssss
def convert_time_string_to_float(time_string):
    return sum([a*b for a,b in zip([3600, 60, 1], map(float, time_string.split(":")))])

# Methods that extract information from the file
def extract_has_finished_constructing_cnf(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[info\] Constructed a total of .* clauses.*")
            if p.match(line):
                return True
    return False

def extract_has_finished(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[info\] Finished constructing DD.*")
            if p.match(line):
                return True
    return False

def extract_finish_time(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[(.*)\]\[info\] Finished constructing DD.*")
            if p.match(line):
                time_string = p.search(line).group(1)
                return convert_time_string_to_float(time_string)
    return -1

def extract_config(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        p = re.compile("Using the following config:([\s\S]*)\[.*\]\[info\] Start Parsing SAS Problem")
        if p.match(content):
            configs = p.search(content).group(1).strip().split("\n")
            return configs
    return []

def extract_total_constructed_clauses(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[info\] Constructed a total of (.*) clauses.*")
            if p.match(line):
                return int(p.search(line).group(1))
    return -1

def extract_conjoined_clauses(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[.*\]\[info\] Conjoined (.*) clauses.*", content)
        if(len(lst) != 0):
            return int(lst[-1])
    return -1

def extract_has_error_while_encoding_to_sat(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[error\] Not a unit cost problem.*")
            if p.match(line):
                return True
    return False

def extract_CUDD_config(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        p = re.compile(".*Printing CUDD statistics\.\.\.([\s\S]*Time for reordering:.*sec)$", flags=re.DOTALL)
        if p.match(content):
            configs = p.search(content).group(1).strip().split("\n")
            return configs
    return []


# Methods that take the information of a dictionary and extracts parts from it or refine the information
def get_time_for_reordering_from_info(info):
    cudd_config = info["cudd_config"]
    for line in cudd_config:
        p = re.compile("Time for reordering: (.*) sec")
        if p.match(line):
            return float(p.search(line).group(1))
    return -1

def get_percentage_spent_reordering_from_info(info):
    finish_time = info["finish_time"]
    reordering_time = get_time_for_reordering_from_info(info)
    if finish_time < 0 or reordering_time < 0:
        return -1
    return 100 * reordering_time/finish_time

def get_peak_number_of_nodes_from_info(info):
    cudd_config = info["cudd_config"]
    for line in cudd_config:
        p = re.compile("Peak number of nodes: (.*)$")
        if p.match(line):
            return int(p.search(line).group(1))
    return -1

def get_peak_number_of_live_nodes_from_info(info):
    cudd_config = info["cudd_config"]
    for line in cudd_config:
        p = re.compile("Peak number of live nodes: (.*)$")
        if p.match(line):
            return int(p.search(line).group(1))
    return -1

def get_number_of_nodes_from_bdd_from_info(info):
    cudd_config = info["cudd_config"]
    for line in cudd_config:
        p = re.compile(".*Number of nodes: (.*), Number of solutions: .*")
        if p.match(line):
            return int(p.search(line).group(1))
    return -1

def get_percentage_of_conjoined_clauses_from_info(info):
    constructed_clauses = info["constructed_clauses"]
    conjoined_clauses = info["conjoined_clauses"]
    return 100 * conjoined_clauses/constructed_clauses



#print(extract_total_constructed_clauses("../test_output/easy_optimal_planDD_test/gripperprob01pddl/planDD_output.txt"))

#print(find_all_output_files("../test_output/easy_optimal_planDD_test/"))

#info_dicts = [compile_information_about_planDD(d,fp) for (d,fp) in find_all_output_files("../test_output/easy_optimal_planDD_test/")]

#print(len(info_dicts[:100]))

#write_all_information_to_file("../test_output/easy_optimal_planDD_test/", "../test_output/info.pkl")

all_dics = read_all_information_from_file("../test_output/info.pkl")
print_information_from_dicts(all_dics)

#i = compile_information_about_planDD("bla", "../test_output/easy_optimal_planDD_test/gripperprob01pddl/planDD_output.txt")
#print(get_peak_number_of_nodes_from_info(i))