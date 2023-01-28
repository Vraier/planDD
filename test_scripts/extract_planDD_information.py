import re
import os
import pickle
import planDD_test_util_problems
import planDD_test_util_commandfile
import planDD_test_util_general as util


# extracts all the possible information from an output file into a dict
def compile_information_about_planDD_into_dic(domain_desc, file_path):
    info = {}

    # general information
    info["domain_desc"] = domain_desc
    info["file_path"] = file_path
    info["config"] = extract_config(file_path)
    insert_config_information_into_info_dict(info)
    info["cudd_config"] = extract_CUDD_config(file_path)

    # how far did the program progress?
    info["has_finished_cnf"] = extract_has_finished_constructing_cnf(file_path)
    info["has_finished"] = extract_has_finished(file_path)
    info["error_while_encoding"] = extract_has_error_while_encoding_to_sat(file_path)

    # information with multiple datapoints
    info["progress_timesteps"] = extract_progress_timesteps(file_path)
    info["progress_conjoin_percent"] = extract_progress_conjoin_percent(file_path)
    info["progress_nodes"] = extract_progress_nodes(file_path)
    info["progress_peak_nodes"] = extract_progress_peak_nodes(file_path)
    info["progress_reorderings"] = extract_progress_reorderings(file_path)
    info["progress_memory"] = extract_progress_memory(file_path)
    info["progress_add_conjoin_permute"] = extract_add_conjoin_permute(file_path)

    info["last_timesteps"] = -99999 if len(info["progress_timesteps"]) == 0 else info["progress_timesteps"][-1]
    info["last_conjoin_percent"] = -99999 if len(info["progress_conjoin_percent"]) == 0 else info["progress_conjoin_percent"][-1]
    info["last_nodes"] = -99999 if len(info["progress_nodes"]) == 0 else info["progress_nodes"][-1]
    info["last_peak_nodes"] = -99999 if len(info["progress_peak_nodes"]) == 0 else info["progress_peak_nodes"][-1]
    info["last_reorderings"] = -99999 if len(info["progress_reorderings"]) == 0 else info["progress_reorderings"][-1]
    info["last_memory"] = -99999 if len(info["progress_memory"]) == 0 else info["progress_memory"][-1]

    # single datapoints (may only be usefull if program finsihed)
    info["finish_time"] = extract_finish_time(file_path)
    info["constructed_clauses"] = extract_total_constructed_clauses(file_path)
    info["constructed_variables"] = extract_constructed_variables(file_path)
    info["finish_colouring"] = extract_colouring_time(file_path)
    info["num_colour_classes"] = extract_num_colours(file_path)
    # parse info again to get more polished information

    info["num_solutions"] = extract_num_solutions(file_path)
    info["query_common_start"] = extract_query_common_start(file_path)
    info["query_common_end"] = extract_query_common_end(file_path)
    info["query_random_start"] = extract_query_random_start(file_path)
    info["query_random_end"] = extract_query_random_end(file_path)

    return info

def extract_num_solutions(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile(".*Number of nodes: .*, Num Variables: .*, Number of solutions: (.*)")
            if p.match(line):
                return float(p.search(line).group(1))
    return -999999

def extract_query_common_start(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p1 = re.compile("\[(.*)\]\[info\] Start finding most common operator for timestep.*")
            if p1.match(line):
                time_string = p1.search(line).group(1)
                return convert_time_string_to_float(time_string)
    return -99999

def extract_query_common_end(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p1 = re.compile("\[(.*)\]\[info\] Finished finding most common operator.*")
            if p1.match(line):
                time_string = p1.search(line).group(1)
                return convert_time_string_to_float(time_string)
    return -99999

def extract_query_random_start(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p1 = re.compile("\[(.*)\]\[info\] Start calculating random set of .* solutions.*")
            if p1.match(line):
                time_string = p1.search(line).group(1)
                return convert_time_string_to_float(time_string)
    return -99999

def extract_query_random_end(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p1 = re.compile("\[(.*)\]\[info\] Found all desired solutions.*")
            if p1.match(line):
                time_string = p1.search(line).group(1)
                return convert_time_string_to_float(time_string)
    return -99999

def compile_information_about_sdd_compiler_into_dic(domain_desc, file_path):
    info = {}
    info["domain_desc"] = domain_desc
    info["file_path"] = file_path

    info["has_finished"] = False
    info["finish_time"] = -1
    info["last_memory"] = -99999
    info["last_nodes"] = -99999
    info["last_pair_count"] = -99999
    info["last_decision_count"] = -99999

    with open(file_path, "r") as f:
        p1 = re.compile("compilation time.*: (.*) sec")
        p2 = re.compile("reading cnf...vars=(.*) clauses=(.*)")
        p3 = re.compile("memory \(free\).*\((.*) MB\)")
        p4 = re.compile("memory \(allocated\).*\((.*) MB\)")
        p5 = re.compile(" sdd size.*: (.*)(?<!dead)$")
        p6 = re.compile(" sdd node count.*: (.*)(?<!dead)$")
        for line in f:
            if p1.match(line):
                info["has_finished"] = True
                info["finish_time"] = float(p1.search(line).group(1))
            if p2.match(line):
                info["constructed_variables"] = int(p2.search(line).group(1))
                info["constructed_clauses"] = int(p2.search(line).group(2))
            if p3.match(line):
                info["last_memory"] = max(info["last_memory"], float(p3.search(line).group(1)))
            if p4.match(line):
                info["last_memory"] = max(info["last_memory"], float(p4.search(line).group(1)))
            if p5.match(line):
                info["last_pair_count"] = int(p5.search(line).group(1).replace(',', ''))
            if p6.match(line):
                info["last_decision_count"] = int(p6.search(line).group(1).replace(',', ''))
                info["last_nodes"] = info["last_decision_count"] + info["last_pair_count"]
                
    return info

# returns a list of tuples that represents paths to output files from planDD
def find_all_output_files(suite_path):
    all_file_paths = []
    for fold in os.listdir(suite_path):
        fold_path = os.path.join(suite_path, fold)
        if os.path.isdir(fold_path):
            for fil in os.listdir(fold_path):
                if "planDD_output" in fil or fil == "output.txt":
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

def write_sdd_compiler_to_file(suite_path, output_path):
    all_files = []
    for fold in os.listdir(suite_path):
        fold_path = os.path.join(suite_path, fold)
        if os.path.isdir(fold_path):
            for fil in os.listdir(fold_path):
                if fil == "sdd_output.txt":
                    file_path = os.path.join(fold_path, fil)
                    all_files.append((fold, file_path))
                    break
    all_dics = []

    for i in range(len(all_files)):
        print("Compile information about testcase ", i+1, " from ", len(all_files))
        domain_desc, file_path = all_files[i]
        dic = compile_information_about_sdd_compiler_into_dic(domain_desc, file_path)
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

# uses the write_all_information_to_file function on every suite in the given folder
def write_whole_set_of_tests_to_file(folder_path):
    for d in os.listdir(folder_path):
        test_folder_path = os.path.join(folder_path, d)
        if os.path.isdir(test_folder_path):
            pickle_name = d + ".pkl"
            pickel_path = os.path.join(folder_path, pickle_name)
            write_all_information_to_file(test_folder_path, pickel_path)

# reads in the dictionaries that were generated by the above method
def read_whole_set_of_tests_from_file(folder_path):
    all_suites = []
    for d in os.listdir(folder_path):
        if ".pkl" in d:
            pickle_file = os.path.join(folder_path, d)
            with open(pickle_file, 'rb') as f:
                all_dics = pickle.load(f)
                all_suites.append(all_dics)
    return all_suites

# time string is in format hh:mm:ss,sssss
def convert_time_string_to_float(time_string):
    return sum([a*b for a,b in zip([3600, 60, 1], map(float, time_string.split(":")))])

# Methods that extract information from the file
def extract_config(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        p = re.compile("Using the following config:([\s\S]*)\[.*\]\[info\] Start Parsing SAS Problem")
        if p.match(content):
            configs = p.search(content).group(1).strip().split("\n")
            return configs
    return []

def extract_CUDD_config(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        p = re.compile(".*Printing CUDD statistics\.\.\.([\s\S]*Time for reordering:.*sec)$", flags=re.DOTALL)
        if p.match(content):
            configs = p.search(content).group(1).strip().split("\n")
            return configs
    return []

def extract_has_finished_constructing_cnf(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[info\] Start constructing DD by lineary adding logic primitives.*")
            if p.match(line):
                return True
    return False

def extract_has_finished(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p1 = re.compile("\[(.*)\]\[info\] Finished constructing DD.*")
            p2 = re.compile("\[(.*)\]\[info\] Finished constructing final DD.*")
            p3 = re.compile("\[(.*)\]\[info\] Printing CUDD statistics\.\.\..*")
            p4 = re.compile("\[(.*)\]\[info\] Printing LibSDD statistics\.\.\..*")
            if p2.match(line) or p3.match(line) or p4.match(line):
                return True
    return False

def extract_finish_time(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p2 = re.compile("\[(.*)\]\[info\] Printing LibSDD statistics.*")
            p3 = re.compile("\[(.*)\]\[info\] Printing CUDD statistics\.\.\..*")
            if p2.match(line):
                time_string = p2.search(line).group(1)
                return convert_time_string_to_float(time_string)
            if p3.match(line):
                time_string = p3.search(line).group(1)
                return convert_time_string_to_float(time_string)
    return -1

def extract_colouring_time(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p3 = re.compile("\[(.*)\]\[info\] Encoder found .* colour classes.*")
            if p3.match(line):
                time_string = p3.search(line).group(1)
                return convert_time_string_to_float(time_string)
    return -99999

def extract_num_colours(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[info\] Encoder found (.*) colour classes.*")
            if p.match(line):
                return int(p.search(line).group(1))
    return -99999

def extract_total_constructed_clauses(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[info\] Ordered a total of (.*) primitives.*")
            if p.match(line):
                return int(p.search(line).group(1))
    return -1

def extract_constructed_variables(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[info\] Constructed (.*) variables during symbol map initialization.*")
            if p.match(line):
                return int(p.search(line).group(1))
    return -1

def extract_has_error_while_encoding_to_sat(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\]\[error\] Not a unit cost problem.*")
            if p.match(line):
                return True
    return False

# returns a list of multiple data points during the execution of the program
def extract_progress_timesteps(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[(.*)\]\[info\] Conjoined .*% of all clauses.*", content)
    return [convert_time_string_to_float(x) for x in lst]

def extract_progress_conjoin_percent(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[.*\]\[info\] Conjoined (\d*)% of all clauses. CUDD stats: #nodes: \d* #peak nodes: \d* #reorderings: \d* #memory bytes: \d*.*", content)
    return [int(x) for x in lst]

def extract_progress_nodes(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[.*\]\[info\] Conjoined \d*% of all clauses. CUDD stats: #nodes: (\d*) #peak nodes: \d* #reorderings: \d* #memory bytes: \d*.*", content)
    return [int(x) for x in lst]

def extract_progress_peak_nodes(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[.*\]\[info\] Conjoined \d*% of all clauses. CUDD stats: #nodes: \d* #peak nodes: (\d*) #reorderings: \d* #memory bytes: \d*.*", content)
    return [int(x) for x in lst]

def extract_progress_reorderings(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[.*\]\[info\] Conjoined \d*% of all clauses. CUDD stats: #nodes: \d* #peak nodes: \d* #reorderings: (\d*) #memory bytes: \d*.*", content)
    return [int(x) for x in lst]

def extract_progress_memory(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[.*\]\[info\] Conjoined \d*% of all clauses. CUDD stats: #nodes: \d* #peak nodes: \d* #reorderings: \d* #memory bytes: (\d*).*", content)
    return [int(x) for x in lst]

def extract_add_conjoin_permute(file_path):
    with open(file_path, "r") as f:
        content = str(f.read())
        lst = re.findall(r"\[(.*)\]\[info\] (?:Adding timestep for t=.* CUDD stats: #nodes: .* #peak nodes: .*|Conjoining two bdds.*|Permuating .* variables)", content)
    return [convert_time_string_to_float(x) for x in lst]

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
        p = re.compile(".*Number of nodes: (.*), Num Variables: .*, Number of solutions: .*")
        if p.match(line):
            return int(p.search(line).group(1))
    return -1

# insert all inforamtion in the config to the info dictionary
def insert_config_information_into_info_dict(info):
    config = info["config"]
    for c in config:
        argument = re.compile("> ([\w|_]*)[\(default\)]*=.*").search(c).group(1) 
        value = c.split("=")[1]
        info["config_"+argument] = value

# extracts information from the output of a fast_downward run on all benchmarks   
# domain_desc: describes which domain and testcase is used (name of the folder used to hold the testcase)      
def downward_write_all_information_to_file(suite_folder="easy_optimal_downward_test", output_file="../test_output/easy_optimal_downward_test.pkl"):
    problem_information = []
    for problem in planDD_test_util_problems.list_all_opt_strips_problems():
        output_folder = planDD_test_util_commandfile.generate_output_directory_name(suite_folder, problem)
        output_path = os.path.join(output_folder, "fd_output.txt")

        if not os.path.isfile(output_path):
            print("Warning", output_path, "does not exist")
            continue
        
        info = {}
        info["domain_desc"] = util.get_sanitized_domain_description(problem["d_name"], problem["p_name"])
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
