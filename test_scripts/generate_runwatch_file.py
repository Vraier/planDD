from os import listdir
from os.path import isfile, join, dirname, basename
import itertools
from itertools import chain, combinations

import planDD_test_util_problems as probs
import planDD_test_util_general as util

SCRIPT_TO_TEST_OUTPUT = "../../test_output"
SUITE_TO_SCRIPT_PATH = "../../../planDD/test_scripts"

DOWNWARD_PATH = "../../downward/fast-downward.py"
PLANDD_PATH = "../../planDD/build/planDD"
SYMK_PATH = "../../symk/fast-downward.py"
FORBIDK_PATH = "../../forbiditerative/plan_topk.sh"
KSTAR_PATH = "../../kstar/fast-downward.py"

# timeout 300 bash -c 'command1 && command2'

# TODO Modify
# one section for symk
# Kstar:
#   - plan_reconstructor 221, 233, 249, 173
#   - fast_downward.py header
# Symk:
#   - plan_database: 134
# fast-downward script for kstar

# TODO, check if everything works
# Think about how to add timeout to forbidk
# use plan.py

# Finds the domain files that must be used for the given problem file
def get_domain_path_from_problem_path(path):
    folder_path = dirname(path)
    prob_name = basename(path)
    onlyfiles = [f for f in listdir(folder_path) if isfile(join(folder_path, f))]
    domains = [f for f in onlyfiles if "domain" in f]

    if len(domains) == 0:
        print("Error, found no domain in", path)
        return ""
    if len(domains) == 1:
        return join(folder_path, domains[0])
    if "domain.pddl" in domains:
        return join(folder_path, "domain.pddl")
    else:
        for d in domains:
            if prob_name in d:
                return join(folder_path, d)
        for d in domains:
            split_prob = prob_name.split("-")[0]
            if len(split_prob) != len(prob_name) and split_prob in d:
                return join(folder_path, d)
        for d in domains:
            split_prob = prob_name.split(".")[0]
            if len(split_prob) != len(prob_name) and split_prob in d:
                return join(folder_path, d)
        print("Error, found no matching domain file", path)
        return ""


# creates the given directory and cd's to it
def create_and_change_directory_command(output_folder):
    mkdir_command = "mkdir -p " + output_folder
    chdir_command = "cd " + output_folder
    whole_command = mkdir_command + " && " + chdir_command
    return whole_command


# used for optimal and naiv track, will execute fd before
def planDD_optimal_command(suite_name, problem, additional_flags):
    sanitized_name = util.get_sanitized_domain_description(
        problem["d_name"], problem["p_name"]
    )
    suite_path = join(SCRIPT_TO_TEST_OUTPUT, suite_name, sanitized_name)
    dir_change_command = create_and_change_directory_command(suite_path)

    problem_path = problem["path"]

    suite_problem_path = join(SUITE_TO_SCRIPT_PATH, problem_path)

    suite_downward_path = join(SUITE_TO_SCRIPT_PATH, DOWNWARD_PATH)
    fd_payload = (
        '{} --sas-file output.sas {} --search "astar(lmcut())" > fd_output.txt'.format(
            suite_downward_path, suite_problem_path
        )
    )
    suite_planDD_path = join(SUITE_TO_SCRIPT_PATH, PLANDD_PATH)
    planDD_payload = "{} --sas_file output.sas {} > output.txt".format(
        suite_planDD_path, additional_flags
    )

    whole_command = "{} && {} && {}".format(
        dir_change_command, fd_payload, planDD_payload
    )
    return whole_command


# does only translate the problem before
def planDD_topK_command(suite_name, problem, num_plans, additional_flags):
    sanitized_name = util.get_sanitized_domain_description(
        problem["d_name"], problem["p_name"]
    )
    suite_path = join(SCRIPT_TO_TEST_OUTPUT, suite_name, sanitized_name)
    dir_change_command = create_and_change_directory_command(suite_path)

    problem_path = problem["path"]

    suite_problem_path = join(SUITE_TO_SCRIPT_PATH, problem_path)

    suite_downward_path = join(SUITE_TO_SCRIPT_PATH, DOWNWARD_PATH)
    fd_payload = "{} --sas-file output.sas --translate {}  > fd_output.txt".format(
        suite_downward_path, suite_problem_path
    )
    suite_planDD_path = join(SUITE_TO_SCRIPT_PATH, PLANDD_PATH)
    planDD_payload = "{} --sas_file output.sas {} --num_plans {} > output.txt".format(
        suite_planDD_path, additional_flags, num_plans
    )

    whole_command = "{} && {} && {}".format(
        dir_change_command, fd_payload, planDD_payload
    )
    return whole_command


# runs fast downward before and finds an initial optimal plan
def planDD_topk_use_fd_command(
    suite_name,
    problem,
    num_plans,
    additional_flags,
):
    sanitized_name = util.get_sanitized_domain_description(
        problem["d_name"], problem["p_name"]
    )
    suite_path = join(SCRIPT_TO_TEST_OUTPUT, suite_name, sanitized_name)
    dir_change_command = create_and_change_directory_command(suite_path)

    problem_path = problem["path"]

    suite_problem_path = join(SUITE_TO_SCRIPT_PATH, problem_path)

    suite_downward_path = join(SUITE_TO_SCRIPT_PATH, DOWNWARD_PATH)
    fd_payload = (
        '{} --sas-file output.sas {} --search "astar(lmcut())" > fd_output.txt'.format(
            suite_downward_path, suite_problem_path
        )
    )
    suite_planDD_path = join(SUITE_TO_SCRIPT_PATH, PLANDD_PATH)
    planDD_payload = "{} --sas_file output.sas {} --num_plans {} > output.txt".format(
        suite_planDD_path, additional_flags, num_plans
    )

    whole_command = "{} && {} && {}".format(
        dir_change_command, fd_payload, planDD_payload
    )
    return whole_command


def symk_topk_command(suite_name, problem, num_plans):
    sanitized_name = util.get_sanitized_domain_description(
        problem["d_name"], problem["p_name"]
    )
    suite_path = join(SCRIPT_TO_TEST_OUTPUT, suite_name, sanitized_name)
    dir_change_command = create_and_change_directory_command(suite_path)

    problem_path = problem["path"]
    domain_path = get_domain_path_from_problem_path(problem_path)

    suite_problem_path = join(SUITE_TO_SCRIPT_PATH, problem_path)
    suite_domain_path = join(SUITE_TO_SCRIPT_PATH, domain_path)

    suite_symk_path = join(SUITE_TO_SCRIPT_PATH, SYMK_PATH)

    symk_payload = '--search "symk-fw(plan_selection=top_k(num_plans={}))"'.format(
        num_plans
    )
    symk_command = "{} {} {} {}".format(
        suite_symk_path, suite_domain_path, suite_problem_path, symk_payload
    )

    whole_command = "{} && {} > output.txt".format(dir_change_command, symk_command)
    return whole_command


def forbidk_topk_command(suite_name, problem, num_plans):
    sanitized_name = util.get_sanitized_domain_description(
        problem["d_name"], problem["p_name"]
    )
    suite_path = join(SCRIPT_TO_TEST_OUTPUT, suite_name, sanitized_name)
    dir_change_command = create_and_change_directory_command(suite_path)

    problem_path = problem["path"]
    domain_path = get_domain_path_from_problem_path(problem_path)

    suite_problem_path = join(SUITE_TO_SCRIPT_PATH, problem_path)
    suite_domain_path = join(SUITE_TO_SCRIPT_PATH, domain_path)

    suite_forbidik_path = join(SUITE_TO_SCRIPT_PATH, FORBIDK_PATH)

    forbidk_command = "{} {} {} {}".format(
        suite_forbidik_path, suite_domain_path, suite_problem_path, num_plans
    )

    whole_command = "{} && {} > output.txt".format(dir_change_command, forbidk_command)
    return whole_command


def kstar_topk_command(suite_name, problem, num_plans):
    sanitized_name = util.get_sanitized_domain_description(
        problem["d_name"], problem["p_name"]
    )
    suite_path = join(SCRIPT_TO_TEST_OUTPUT, suite_name, sanitized_name)
    dir_change_command = create_and_change_directory_command(suite_path)

    problem_path = problem["path"]
    domain_path = get_domain_path_from_problem_path(problem_path)

    suite_problem_path = join(SUITE_TO_SCRIPT_PATH, problem_path)
    suite_domain_path = join(SUITE_TO_SCRIPT_PATH, domain_path)

    suite_kastar_path = join(SUITE_TO_SCRIPT_PATH, KSTAR_PATH)

    kastar_payload = '--search "kstar(blind(),k={},verbosity=silent)"'.format(num_plans)
    kastar_command = "{} {} {} {}".format(
        suite_kastar_path, suite_domain_path, suite_problem_path, kastar_payload
    )

    whole_command = "{} && {} > output.txt".format(dir_change_command, kastar_command)
    return whole_command


def generate_topk_runwatch_command_file(problems, suites):
    with open("all_commands.txt", "w") as runwatch_file:
        num_commands = 0
        for planner, name, k, flags in suites:
            suite_name = "{}_k{}".format(name, k)
            for problem in problems:
                task_command = ""
                if planner == "planDD":
                    task_command = planDD_topK_command(suite_name, problem, k, flags)
                elif planner == "planDDOptimal":
                    task_command = planDD_optimal_command(suite_name, problem, flags)
                elif planner == "planDDUseFD":
                    task_command = planDD_topk_use_fd_command(
                        suite_name, problem, k, flags
                    )
                elif planner == "symk":
                    task_command = symk_topk_command(suite_name, problem, k)
                elif planner == "kstar":
                    task_command = kstar_topk_command(suite_name, problem, k)
                elif planner == "forbidk":
                    task_command = forbidk_topk_command(suite_name, problem, k)
                # runwatch_file.write("{} {}\n".format(str(num_commands), task_command))
                runwatch_file.write("{}\n".format(task_command))
                num_commands += 1
        print("Generated a total of", num_commands, "commands")


planDD_topK_flags = "--linear --timesteps -1 --clause_order_custom --var_order_custom --build_order rympec:: --binary_encoding --binary_exclude_impossible --binary_variables"
planDD_topK_use_fd_flags = "--linear --timesteps -1 --clause_order_custom --var_order_custom --build_order rympec:: --binary_encoding --binary_exclude_impossible --binary_variables --use_fd"
planDD_topK_restart_flags = "--linear --timesteps -1 --clause_order_custom --var_order_custom --build_order igrymx:pec: --binary_encoding --binary_exclude_impossible --binary_variables --restart"
planDD_topK_restart_use_fd_flags = "--linear --timesteps -1 --clause_order_custom --var_order_custom --build_order igrymx:pec: --binary_encoding --binary_exclude_impossible --binary_variables --restart --use_fd"

# better flag for restarting: igx:rympec:
planDD_improved_restart = "--linear --timesteps -1 --clause_order_custom --var_order_custom --build_order igx:rympec: --binary_encoding --binary_exclude_impossible --binary_variables --restart"
planDD_improved_restart_use_fd = "--linear --timesteps -1 --clause_order_custom --var_order_custom --build_order igx:rympec: --binary_encoding --binary_exclude_impossible --binary_variables --restart --use_fd"

# naiv, optimal, restarting and incremental (both with fd) suites, 14/12/2022
planDD_naiv_bdd_14_12 = (
    "--build_bdd_naiv --build_order igrympecx:: --clause_order_custom"
)
planDD_optimal_bdd_14_12 = "--build_bdd --linear --timesteps 1 --use_fd --build_order igx:rympec: --variable_order x:voh --clause_order_custom --var_order_custom --binary_encoding --binary_variables --binary_exclude_impossible"
planDD_restart_14_12 = "--build_bdd --linear --timesteps -1 --clause_order_custom --var_order_custom --build_order igx:rympec: --binary_encoding --binary_exclude_impossible --binary_variables --restart --use_fd"
planDD_incremental_14_12 = "--build_bdd --linear --timesteps -1 --clause_order_custom --var_order_custom --build_order rympec:: --binary_encoding --binary_exclude_impossible --binary_variables --use_fd"

currentK = 10**9

opt_strip_unit_cost_problems = probs.list_all_opt_strips_unitcost_problems()
random_easy_downard_probs = probs.select_random_set_from_downward_solved_problems(200)


# planner type, suite name, num plan, additional flags
suites = [
    # on 137 with 600 sec timeout 60 cores
    # ("planDD", "planDDTopK", currentK, planDD_topK_flags),
    # ("planDDUseFD", "planDDUseFD", currentK, planDD_topK_use_fd_flags),
    # ("planDD", "planDDTopKRestart", currentK, planDD_topK_restart_flags),
    # ("planDDUseFD", "planDDRestartUseFD", currentK, planDD_topK_restart_use_fd_flags),
    # ("symk", "symk", currentK, ""),
    # ("kstar", "kstar", currentK, ""),
    ##### ("forbidk", 10000000, 300), # i dont do this for now because it would write too many files
    # on 135 600 sec timeout 60 cores (sic)
    # ("planDD", "planDD_31_11", currentK, planDD_topK_flags),
    # ("planDD", "planDDFixedRestart", currentK, planDD_improved_restart),
    # ("planDDUseFD", "planDDFixedRestartUseFD", currentK, planDD_improved_restart_use_fd),
    # naiv, optimal, and restarting suites
    ("planDDOptimal", "planDDNaivBdd_14_12", currentK, planDD_naiv_bdd_14_12),
    ("planDDOptimal", "planDDOptimalBdd_14_12", currentK, planDD_optimal_bdd_14_12),
    ("planDDUseFD", "planDDRestart_14_12", currentK, planDD_restart_14_12),
    ("planDDUseFD", "planDDIncremental_14_12", currentK, planDD_incremental_14_12),
]


# T1 clause order suites
def generate_all_conjoin_orders():
    all_orders = []
    single_timesetps = [
        "i",  # initial state
        "g",  # goal
    ]
    multi_timesteps = [
        "rym",  # all variable and action mutexes
        "pe",  # preconditions and effects
        "c",  # changing atoms
    ]
    # choose which parts not to interleave but order disjoint
    # (interleaving only one or zero part(s) does are edge cases, i dont do interleaved parts of size 0)
    powerset = list(
        chain.from_iterable(
            combinations(multi_timesteps, r) for r in range(len(multi_timesteps))
        )
    )

    for subset in powerset:
        local_single = single_timesetps + list(subset) + ["x"]
        local_multi = [x for x in multi_timesteps if x not in subset]

        single_permutations = list(itertools.permutations(local_single))
        multi_permutations = list(itertools.permutations(local_multi))

        for s in single_permutations:
            for p in multi_permutations:
                new_order = "".join(s) + ":" + "".join(p) + ":"
                all_orders.append(new_order)

    return all_orders


clause_order_suites = []
clause_orders = generate_all_conjoin_orders()
for i in range(len(clause_orders)):
    ord = clause_orders[i]
    clause_order_suites.append(
        ("planDDOptimal", 
        "T01_11_01_ord_" + str(i), 
        currentK, 
        "--build_bdd --linear --use_fd --build_order {} --clause_order_custom".format(ord))
    )

# parallel --jobs X --timeout 60 :::: all_commands.txt
#print("Num problems:", len(random_easy_downard_probs))
#print("Num configs:", len(clause_order_suites))
#generate_topk_runwatch_command_file(random_easy_downard_probs, clause_order_suites)


# T2 var order suites
def generate_variables_orders():
    return ["vox:h", "ovx:h", "x:voh", "x:ovh"]

var_order_suites = []
var_orders = generate_variables_orders()
for i in range(len(var_orders)):
    ord = var_orders[i]
    var_order_suites.append(
        ("planDDOptimal", 
        "T02_11_01_ord_" + str(i), 
        currentK, 
        "--build_bdd --linear --use_fd --build_order igx:rympec: --clause_order_custom --var_order_custom --variable_order ".format(ord))
    )

# parallel --jobs X --timeout 60 :::: all_commands.txt
print("Num problems:", len(random_easy_downard_probs))
print("Num configs:", len(var_order_suites))
generate_topk_runwatch_command_file(random_easy_downard_probs, var_order_suites)


# T3 different building algorithms