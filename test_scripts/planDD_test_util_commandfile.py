import os

PATH_TO_PLANDD = "../main"
PATH_TO_DOWNWARD = "../../downward/fast-downward.py"

# template for the command lins string. Vars with $ get substituted
STANDART_PLANDD_COMMANDLINE_STRING = ""
STANDART_PLANDD_COMMANDLINE_STRING += "timeout $timeout "
STANDART_PLANDD_COMMANDLINE_STRING += os.path.join("../../", PATH_TO_PLANDD)
STANDART_PLANDD_COMMANDLINE_STRING += " --sas_file output.sas "
STANDART_PLANDD_COMMANDLINE_STRING += " --$mode "
STANDART_PLANDD_COMMANDLINE_STRING += " --timesteps $timesteps "
STANDART_PLANDD_COMMANDLINE_STRING += " --build_order $build_order "
STANDART_PLANDD_COMMANDLINE_STRING += " $addition_flags "
STANDART_PLANDD_COMMANDLINE_STRING += " > planDD_output.txt "

STANDART_DOWNWARD_COMMANDLINE_STRING = ""
STANDART_DOWNWARD_COMMANDLINE_STRING += os.path.join("../../", PATH_TO_DOWNWARD)
STANDART_DOWNWARD_COMMANDLINE_STRING += " --sas-file output.sas "
STANDART_DOWNWARD_COMMANDLINE_STRING += " --translate-time-limit $downward_timeout "
STANDART_DOWNWARD_COMMANDLINE_STRING += " --translate $problem_path "
STANDART_DOWNWARD_COMMANDLINE_STRING += " > fd_output.txt"

# Takes a list of commandline calls and generates a file for the parallel tool
# example to call the all_commands file with parallel
# parallel --jobs 2 :::: all_commands.txt"
def generate_parallel_file_from_calls(all_commandline_calls): 
    with open("all_commands.txt", "w") as parallel_file:
        for commandline_call in all_commandline_calls:
            parallel_file.write(commandline_call + "\n")

# takes a list of problems and agument maps
# constructs all the necessary combinations to run the config on the problem
# input format is:
# prob = (domain_name, prob_name, prob_path)
# argument = (suite_name, planDD_map, downward_map)
def generate_command_calls(list_of_problems, list_of_arguments):
    all_commandline_calls = []
    for argument in list_of_arguments:
        for prob in list_of_problems:
            (suite_name, planDD_map, downward_map) = argument
            (_, _, prob_path) = prob
            output_folder = generate_output_directory_name(suite_name, prob)
            commandline_call = construct_complete_call(output_folder, prob_path, planDD_map, downward_map)
            all_commandline_calls.append(commandline_call)
    return all_commandline_calls



# returns a samitized name for an output directory, given a problem
# suite folder = easy_optimal_downward_test/, 
# input format: (domain, problem, problem_path)
# return output_path
def generate_output_directory_name(suite_name, problem):
    (d, p, _) = problem
    
    unsanitized_name = d + p
    sanitized_name = "".join(x for x in unsanitized_name if x.isalnum())
    test_instance_path = os.path.join("../test_output/", suite_name, sanitized_name)
        
    return test_instance_path



# inserts the arguments of the map into the commandline string
def apply_argument_map_to_commandline_string(commandline_string, argument_map):
    new_string = commandline_string
    for key in argument_map:
        if not key in new_string:
            print("Warning, cant replace key:", key, " in", new_string)
        new_string = new_string.replace(key, argument_map[key], 1)
    return new_string

# constructs the call for the planDD program. Will also call the downward translator
# needs the length of an optimal plan for the problem
# problem path is relative from location of test_script folder
def construct_complete_call(output_folder, problem_path, planDD_argument_map, downward_argument_map):
    mkdir_command = "mkdir -p " + output_folder
    chdir_command = "cd " + output_folder
    new_downward_dic = dict(downward_argument_map)
    new_downward_dic["$problem_path"] = problem_path
    downward_translate_command = apply_argument_map_to_commandline_string(STANDART_DOWNWARD_COMMANDLINE_STRING, new_downward_dic)
    planDD_command = apply_argument_map_to_commandline_string(STANDART_PLANDD_COMMANDLINE_STRING, planDD_argument_map)
    whole_command = mkdir_command + " && " + chdir_command + " && " + downward_translate_command + " && " + planDD_command
    return whole_command