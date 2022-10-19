import os
import suites
import extract_planDD_information
import planDD_test_util_general as util

# lists the path to all domains and problems that conatin optimal strips problems
# format: (domain, problem, full_path)
def list_all_opt_strips_problems():
    all_problems = []
    for domain in suites.suite_optimal_strips():
        domain_path = os.path.join(util.PATH_TO_BENCHMARKS, domain)
        for f in os.listdir(domain_path):
            file_path = os.path.join(domain_path, f)
            if os.path.isfile(file_path) and not "domain" in f and ".pddl" in f:
                all_problems.append({
                    "d_name" : domain,
                    "p_name" : f,
                    "path" : file_path,
                    "plan_length" : -1,
                })
    return all_problems

def list_all_opt_strips_unitcost_problems():
    all_problems = suites.suite_optimal_strips()
    unit_cost_problem = [p for p in all_problems if suites.TAG_HAS_ONLY_UNIT_COST_ACTIONS in suites.DOMAIN_TO_TAGS[p]]
    result = []
    for domain in unit_cost_problem:
        domain_path = os.path.join(util.PATH_TO_BENCHMARKS, domain)
        for f in os.listdir(domain_path):
            file_path = os.path.join(domain_path, f)
            if os.path.isfile(file_path) and not "domain" in f and ".pddl" in f:
                result.append({
                    "d_name" : domain,
                    "p_name" : f,
                    "path" : file_path,
                    "plan_length" : -1,
                })
    return result

def list_all_downward_solved_problems():
    downward_dics = extract_planDD_information.downward_read_all_information_from_file()
    all_problems = list_all_opt_strips_problems()

    # Allows to quickly find the information about a testcase from an older run
    downward_domain_to_dic = {}
    for d in downward_dics:
        downward_domain_to_dic[d["domain_desc"]] = d

    # only select problems that were solved by downward (astar(lmcut()))
    # also append the length of an optimal plan
    filtered_problems = []
    for p in all_problems:
        problem_domain_desc = util.get_sanitized_domain_description(p["d_name"], p["p_name"])
        if downward_domain_to_dic[problem_domain_desc]["has_finished"]:
            prob_with_opt_length = dict(p)
            prob_with_opt_length["plan_length"] = downward_domain_to_dic[problem_domain_desc]["path_length"]
            filtered_problems.append(prob_with_opt_length)
    
    return filtered_problems


# returns only the problems that were solved during the benchmark generation run
def list_all_easy_opt_strips_problems():  
    planDD_dics = extract_planDD_information.read_all_information_from_file("../test_output/easy_optimal_planDD_test.pkl")
    downward_dics = extract_planDD_information.downward_read_all_information_from_file()
    all_problems = list_all_opt_strips_problems()
    
    # Allows to quickly find the information about a testcase from an older run
    planDD_domain_to_dic = {}
    downward_domain_to_dic = {}
    for d in planDD_dics:
        planDD_domain_to_dic[d["domain_desc"]] = d
    for d in downward_dics:
        downward_domain_to_dic[d["domain_desc"]] = d

    # planDD solved it or it has only few clauses (20000) or few (1000) variables
    def is_easy_problem(domain_desc):
        if not domain_desc in planDD_domain_to_dic: # this is the case if the problem was not finished by downward
            return False
        if planDD_domain_to_dic[domain_desc]["error_while_encoding"]:
            return False
        if planDD_domain_to_dic[domain_desc]["has_finished"]:
            return True
        if planDD_domain_to_dic[domain_desc]["has_finished_cnf"] and planDD_domain_to_dic[domain_desc]["constructed_clauses"] < 20000:
            return True
        if planDD_domain_to_dic[domain_desc]["has_finished_cnf"] and planDD_domain_to_dic[domain_desc]["constructed_variables"] < 1000:
            return True
        return False

    # only select problems that were solved by planDD
    # also append the length of an optimal plan
    filtered_problems = []
    for p in all_problems:
        # check if the planDD approach solved it before
        problem_domain_desc = util.get_sanitized_domain_description(p["d_name"], p["p_name"])
        if is_easy_problem(problem_domain_desc):
            prob_with_opt_length = dict(p)
            prob_with_opt_length["plan_length"] = downward_domain_to_dic[problem_domain_desc]["path_length"]
            filtered_problems.append(prob_with_opt_length)
            
    return filtered_problems