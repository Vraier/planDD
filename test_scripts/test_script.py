import itertools
from itertools import chain, combinations
from os import system

planning_to_dd_path = "../planning_to_DD/main"

commandline_string = "timeout $timeout " + planning_to_dd_path + " --sas_file $sas_file --$mode --timesteps $timesteps --build_order $build_order > $output_folder/$output_file"

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


def generate_all_interleaved_argument_maps():
    all_maps = []
    standart_argument_map = {
        "$mode" : "build_bdd",
        "$timeout" : "80s",
        "$sas_file" : "output.sas",
        "$timesteps" : "11",
        "$output_folder" : "interleaved_bdd"
    }
    all_interleaved_orders = generate_all_interleaved_orders()

    for i in range(len(all_interleaved_orders)):
        new_map = dict(standart_argument_map)
        new_map["$build_order"] = all_interleaved_orders[i]
        new_map["$output_file"] = "test_run" + str(i) + ".txt"
        all_maps.append(new_map)

    return all_maps
    



def apply_argument_map_to_commandline_string(commandline_string, argument_map):
    new_string = commandline_string
    for key in argument_map:
        new_string = new_string.replace(key, argument_map[key], 1)
    return new_string

def generate_all_argument_maps():
    all_maps = []
    standart_argument_map = {
        "$mode" : "build_sdd",
        "$timeout" : "180s",
        "$sas_file" : "output.sas",
        "$timesteps" : "11",
        "$output_folder" : "sdd_try"
    }
    all_new_orders = generate_all_disjoint_clause_orders()

    for i in range(len(all_new_orders)):
        new_map = dict(standart_argument_map)
        new_map["$build_order"] = all_new_orders[i]
        new_map["$output_file"] = "test_run" + str(i) + ".txt"
        all_maps.append(new_map)

    return all_maps

def generate_all_commandline_strings(agument_maps):
    return [apply_argument_map_to_commandline_string(commandline_string, m) for m in agument_maps]


def execute_all_commands(command_list):
    with open("all_commands.sh", "w") as f:
        f.write("#!/bin/sh\n")
        for command in command_list:
            f.write("%s\n" %command)

    system("chmod +x all_commands.sh")
    system("./all_commands.sh")

    





#print(all_orders)

#print(apply_argument_map_to_commandline_string(command_line_string, argument_map))

#all_commands = generate_all_commandline_strings(generate_all_argument_maps())[93:]
all_commands = generate_all_commandline_strings(generate_all_interleaved_argument_maps())
print(all_commands)

execute_all_commands(all_commands)

#o = generate_all_interleaved_orders()
#print(o)

#print(len(o))