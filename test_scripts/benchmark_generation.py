import suites
import os
import re

path_to_benchmarks = "../../downward-benchmarks/"
path_to_downward = "../../downward/fast-downward.py"

# lists the path to all domains and problems that conatin optimal strips problems
# format: (domain, problem, full_path)
def list_all_problems():
    all_problems = []
    for domain in suites.suite_optimal_strips():
        domain_path = os.path.join(path_to_benchmarks, domain)
        for f in os.listdir(domain_path):
            file_path = os.path.join(domain_path, f)
            if os.path.isfile(file_path) and not "domain" in f and ".pddl" in f:
                all_problems.append((domain, f, file_path))
    return all_problems

# returns a samitized name for an output directory, given a problem
# suite folder = easy_optimal_downward_test/, 
# input format: (doman, problem, problem_path)
# return output_path
def generate_output_directory_name(suite_folder, problem):
    (d, p, _) = problem
    
    unsanitized_name = d + p
    sanitized_name = "".join(x for x in unsanitized_name if x.isalnum())
    test_instance_path = os.path.join("../test_output/", suite_folder, sanitized_name)
        
    return test_instance_path

# problem_file is relative to test_scripts directory
# overall timelimit of 3min
def construct_downward_call(output_folder, problem_path):
    mkdir_command = "mkdir -p " + output_folder
    chdir_command = "cd " + output_folder
    downward_command =  os.path.join("../../", path_to_downward) + " --overall-time-limit 180 " + os.path.join("../../", problem_path) + " --search \"astar(lmcut())\" > fd_output.txt"
    
    whole_command = mkdir_command + " && " + chdir_command + " && " + downward_command
    return whole_command

# TODO: implement
def construct_DD_call(output_folder, problem_path, plan_length):
    pass


# constructs all the calls tha run dast downward on the opt strips benchmarks
def generate_downward_calls(suite_folder="easy_optimal_downward_test"):
    all_problems = list_all_problems()
    all_commandline_calls = []    
    
    for i in range(len(all_problems)):
        _, _, problem_path = all_problems[i]
        #print(problem_path)
        output_folder = generate_output_directory_name(suite_folder, all_problems[i])
        downward_call = construct_downward_call(output_folder, problem_path)
        all_commandline_calls.append(downward_call)
        
    return all_commandline_calls

# uses the results of the fast downward run to determine path_leght and run DD approach
# will only construct calls to problems that fast-downward already solved
def generate_DD_calls():
    downward_problem_information = extract_information_from_all_problems()
    all_commandline_calls = []
    for info in downward_problem_information:
        if info["has_finished"]:
            DD_call = construct_DD_call()
            all_commandline_calls.append(DD_call)
            
    return all_commandline_calls

# Takes a list of commandline calls and generates a file for the parallel tool
# example to call the all_commands file with parallel
# parallel --jobs 2 :::: all_commands.txt"
def generate_parallel_file_from_calls(all_commandline_calls): 
    with open("all_commands.txt", "w") as parallel_file:
        for commandline_call in all_commandline_calls:
            parallel_file.write(commandline_call + "\n")

# extracts information from the output of a fast_downward run on all benchmarks         
def extract_information_from_all_problems(suite_folder="easy_optimal_downward_test"):
    problem_information = []
    for problem in list_all_problems():
        d, p, problem_path = problem
        output_folder = generate_output_directory_name(suite_folder, problem)
        output_path = os.path.join(output_folder, "fd_output.txt")

        if not os.path.isfile(output_path):
            print("Warning", output_path, "does not exist")
            continue
        
        info = dict()
        info["domain"] = d
        info["problem"] = p
        info["problem_path"] = problem_path
        info["output_path"] = output_path

        info["has_finished"] = extract_has_finished(output_path)
        info["finish_time"] = extract_finish_time(output_path)
        info["path_legth"] = extract_plan_length(output_path)
        
        problem_information.append(info)
        
    return problem_information

# Helper methods to extract information from the downward output     
def extract_has_finished(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[.*\] Solution found!")
            if p.match(line):
                return True
    return False

def extract_finish_time(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[t=(.*)s,.*\] Solution found!")
            if p.match(line):
                path_length = int(p.search(line).group(1))
                return path_length
    return -1
    
def extract_plan_length(file_path):
    with open(file_path, "r") as f:
        for line in f:
            p = re.compile("\[t=.*s,.*\] Plan length: (.*) step\(s\).")
            if p.match(line):
                path_length = int(p.search(line).group(1))
                return path_length
    return -1


#generate_parallel_file_from_calls(generate_downward_calls())
downward_search_infos = extract_information_from_all_problems()