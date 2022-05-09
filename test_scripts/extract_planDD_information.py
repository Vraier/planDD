import re
import os
import pickle
import planDD_test_util_problems
import planDD_test_util_commandfile

# extracts all the possible information from an output file into a dict
# domain_desc: describes which domain and tescase is used (name of the folder used to hold the testcase)
def compile_information_about_planDD_into_dic(domain_desc, file_path):
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

# returns a list of tuples that represents paths to output files from planDD
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

# takes a suite_path and generated dictionaries that conatins information of the tescases
# these dicst get written to file for later reuse
def write_all_information_to_file(suite_path, output_path):
    all_files = find_all_output_files(suite_path)
    all_dics = []

    for i in range(len(all_files)):
        print("Compile information about testcase ", i+1, " from ", len(all_files))
        domain_desc, file_path = all_files[i]
        dic = compile_information_about_planDD_into_dic(domain_desc, file_path)
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

# TODO: fix the filenames
# extracts information from the output of a fast_downward run on all benchmarks         
def downward_write_all_information_to_file(output_file="../test_output/easy_optimal_downward_test.pkl", suite_folder="easy_optimal_downward_test"):
    problem_information = []
    for problem in planDD_test_util_problems.list_all_opt_strips_problems():
        output_folder = planDD_test_util_commandfile.generate_output_directory_name(suite_folder, problem)
        output_path = os.path.join(output_folder, "fd_output.txt")

        if not os.path.isfile(output_path):
            print("Warning", output_path, "does not exist")
            continue
        
        info = {}
        info["domain"] = problem["d_name"]
        info["problem"] = problem["p_name"]
        info["problem_path"] = problem["path"]
        info["output_path"] = output_path

        info["has_finished"] = downward_extract_has_finished(output_path)
        info["finish_time"] = downward_extract_finish_time(output_path)
        info["path_length"] = downward_extract_plan_length(output_path)
        
        problem_information.append(info)
    
    with open(output_file, "wb") as f:
        pickle.dump(problem_information, f) 
        
# Reads in the dicitonaries that were written to file eralier
def downward_read_all_information_from_file(pickle_file="../test_output/easy_optimal_downward_test.pkl"):
    all_dics = []
    with open(pickle_file, 'rb') as f:
        all_dics = pickle.load(f)
    return all_dics
         
# Helper methods to extract information from the downward output     
def downward_extract_has_finished(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[t=.*s,.*\] Solution found!")
            if p.match(line):
                return True
    return False

def downward_extract_finish_time(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[t=(.*)s,.*\] Solution found!")
            if p.match(line):
                path_length = float(p.search(line).group(1))
                return path_length
    return -1
    
def downward_extract_plan_length(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[t=.*s,.*\] Plan length: (.*) step\(s\).")
            if p.match(line):
                path_length = int(p.search(line).group(1))
                return path_length
    return -1
