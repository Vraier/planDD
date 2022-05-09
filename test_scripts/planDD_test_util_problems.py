import os
import suites
import extract_planDD_information

PATH_TO_BENCHMARKS = "../../downward-benchmarks/"

# lists the path to all domains and problems that conatin optimal strips problems
# format: (domain, problem, full_path)
def list_all_opt_strips_problems():
    all_problems = []
    for domain in suites.suite_optimal_strips():
        domain_path = os.path.join(PATH_TO_BENCHMARKS, domain)
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

# TODO: extend it to also include problems with low number of vars/clauses
# returns only the problems that were solved during the benchmark generation run
def list_all_easy_opt_strips_problems():
    def get_domain_desc_from_prob(problem):
        "".join(x for x in problem["d_name"]+problem["p_name"] if x.isalnum())
        
    planDD_dics = extract_planDD_information.read_all_information_from_file("../test_output/easy_optimal_downward_test.pkl")
    downward_dics = extract_planDD_information.downward_read_all_information_from_file()
    all_problems = list_all_opt_strips_problems()
    
    # Allows to quickly find the information about a testcase from an older run
    planDD_domain_to_dic = {}
    downward_domain_to_dic = {}
    for d in planDD_dics:
        if d["has_finished"]:
            planDD_domain_to_dic[d["domain_desc"]] = d
    for d in downward_dics:
        if d["has_finished"]:
            downward_domain_to_dic[d["domain_desc"]] = d

    # only select problems that were solved by planDD
    # also append the length of an optimal plan
    filtered_problems = []
    for p in all_problems:
        # check if the planDD approach solved it before
        if get_domain_desc_from_prob(p) in planDD_domain_to_dic:
            prob_with_opt_length = dict(p)
            prob_with_opt_length["plan_length"] = downward_domain_to_dic[get_domain_desc_from_prob(p)]["path_length"]
            filtered_problems.append(prob_with_opt_length)
            
    return filtered_problems