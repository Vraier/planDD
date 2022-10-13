from os import listdir
from os.path import isfile, join, dirname, basename

import planDD_test_util_problems as probs
import planDD_test_util_general as util

SCRIPT_TO_TEST_OUTPUT = "../../test_output"
SUITE_TO_SCRIPT_PATH = "../../../planDD/test_scripts"

SYMK_PATH = "../../symk/fast-downward.py"

TIMEOUT = 3000 # timeout in seconds

def get_domain_path_from_problem_path(path):
    folder_path = dirname(path)
    prob_name = basename(path)
    onlyfiles = [f for f in listdir(folder_path) if isfile(join(folder_path, f))]
    
    domains = [f for f in onlyfiles if f == "domain.pddl"]
    
    if len(domains) == 0:
        print("Error, found no domain")
        return ""
    if len(domains) == 1:
        return join(folder_path, domains[0])
    else:
        for d in domains:
            if prob_name in d:
                return join(folder_path, d)
        print("Error, found no matching domain file")
        return ""
            

def create_and_change_directory_command(output_folder):
    mkdir_command = "mkdir -p " + output_folder
    chdir_command = "cd " + output_folder

    whole_command = mkdir_command + " && " + chdir_command
    return whole_command

def planDD_topk_command(problem, num_plans):
    pass

def symk_topk_command(suite_name, problem, num_plans):    
    sanitized_name = util.get_sanitized_domain_description(problem["d_name"], problem["p_name"])
    suite_path = join(SCRIPT_TO_TEST_OUTPUT, suite_name, sanitized_name)
    dir_change_command = create_and_change_directory_command(suite_path)
    
    problem_path = problem["path"]
    domain_path = get_domain_path_from_problem_path(problem_path)
    
    suite_problem_path = join(SUITE_TO_SCRIPT_PATH, problem_path)
    suite_domain_path = join(SUITE_TO_SCRIPT_PATH, domain_path)
    
    suite_symk_path = join(SUITE_TO_SCRIPT_PATH, SYMK_PATH)
    
    symk_payload = "--search \"symk-fw(plan_selection=top_k(num_plans={}))\"".format(num_plans)
    symk_command = "{} {} {} {}".format(suite_symk_path, suite_domain_path, suite_problem_path, symk_payload)

    whole_command = "{} && {}".format(dir_change_command, symk_command)    
    return whole_command    
    
print(symk_topk_command("testi", probs.list_all_opt_strips_problems()[0], 10))

