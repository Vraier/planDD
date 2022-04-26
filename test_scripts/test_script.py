import itertools
import plot_data
from itertools import chain, combinations
from os import system

planning_to_dd_path = "../planDD/main"

def generate_all_disjoint_clause_orders():
    clause_groups = [
        "i",        # initial state
        "g",        # goal
        "rtyum",    # all variable and action mutexes
        "pe",       # preconditions and effects
        "c"         # changing atoms
    ]
    all_permutations = list(itertools.permutations(clause_groups))
    all_orders = ["".join(p) + "x:" for p in all_permutations]
    return all_orders

def generate_all_interleaved_orders():
    all_orders = []
    single_timesetps = [
        "i",        # initial state
        "g",        # goal
    ]
    multi_timesteps = [
        "rtyum",    # all variable and action mutexes
        "pe",       # preconditions and effects
        "c"         # changing atoms
    ]
    # choose which parts not to interleave 
    # (interleaving only one part does not make sense, i try it anyways for sanity check, i dont do interleaved parts of size 0)
    powerset = list(chain.from_iterable(combinations(multi_timesteps, r) for r in range(len(multi_timesteps))))
    
    for subset in powerset:
        local_single = single_timesetps + list(subset) + ["x"]
        local_multi = [x for x in multi_timesteps if x not in subset]

        single_permutations = list(itertools.permutations(local_single))
        multi_permutations = list(itertools.permutations(local_multi))

        for s in single_permutations:
            for p in multi_permutations:
                new_order = "".join(s) + ":" + "".join(p)
                all_orders.append(new_order)
    
    return all_orders

# reverses an order string. E.g
# ixpec:rtyum -> cepxi:muytr
# This is necessary because the --revers_order flag reverses the whole vector of clauses.
# it allows us to conjoin clauses with higher timesteps first
def revers_order(order_string):
    s1, s2 = order_string.split(":")[0], order_string.split(":")[1]
    return s1[::-1] + ":" + s2[::-1]

# Adds the conjoin orders and corresonding filename to the defualt_argument map
def generate_argmument_maps_for_conjoin_orders(default_arguments, conjoin_orders):
    all_maps = []

    for i in range(len(conjoin_orders)):
        new_map = dict(default_arguments)
        new_map["$build_order"] = conjoin_orders[i]
        new_map["$output_file"] = "test_run" + str(i) + ".txt"
        all_maps.append(new_map)

    return all_maps   


# inserts the arguments in the map into the default string
def apply_argument_map_to_commandline_string(commandline_string, argument_map):
    new_string = commandline_string
    for key in argument_map:
        new_string = new_string.replace(key, argument_map[key], 1)
    return new_string

# converts each argument map into a commandline string
def generate_all_commandline_strings(commandline_string, agument_maps):
    return [apply_argument_map_to_commandline_string(commandline_string, m) for m in agument_maps]


# executes a list of commands. The commands get written into a all_commands.sh file
def execute_all_commands(command_list):
    with open("all_commands.sh", "w") as f:
        f.write("#!/bin/sh\n")
        for command in command_list:
            f.write("%s\n" %command)

    system("chmod +x all_commands.sh")
    system("./all_commands.sh")

    
standart_argument_map = {
    "$mode" : "build_bdd",
    "$timeout" : "80s",
    "$sas_file" : "output.sas",
    "$timesteps" : "11",
    "$output_folder" : "../test_script/interleaved_bdd",
    "$addition_flags" : "",
}
standart_commandline_string = "timeout $timeout " + planning_to_dd_path + " --sas_file $sas_file --$mode --timesteps $timesteps --build_order $build_order $addition_flags > $output_folder/$output_file"


#all_argument_maps = generate_argmument_maps_for_conjoin_orders(standart_argument_map, generate_all_interleaved_orders())
#all_commands = generate_all_commandline_strings(standart_commandline_string, all_argument_maps)

# Test Run for including mutex constraints
best_orders = [plot_data.unsimplify_order_string(x) for x in plot_data.find_all_finished_orders("../test_output/interleaved_bdd")]
mutex_standart_map = standart_argument_map.copy()
mutex_standart_map["$addition_flags"] = "--include_mutex"
mutex_standart_map["$output_folder"] = "../test_script/include_mutex"
mutex_arguments_maps = generate_argmument_maps_for_conjoin_orders(mutex_standart_map, best_orders)
all_commands = generate_all_commandline_strings(standart_commandline_string, mutex_arguments_maps)

"""
# Test Run for trying to reverse the best orders
best_orders = [plot_data.unsimplify_order_string(x) for x in plot_data.find_all_finished_orders("../test_output/interleaved_bdd")]
reverse_standart_map = standart_argument_map.copy()
reverse_standart_map["$addition_flags"] = "--revers_order"
reverse_standart_map["$output_folder"] = "../test_script/reverse_order"
reverse_argument_maps = generate_argmument_maps_for_conjoin_orders(reverse_standart_map, best_orders)
all_commands = generate_all_commandline_strings(standart_commandline_string, reverse_argument_maps)
"""

print(all_commands)
#execute_all_commands(all_commands)
