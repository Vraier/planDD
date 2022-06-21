import planDD_test_util_problems as problems
import planDD_test_util_arguments as arguments
import planDD_test_util_commandfile as commandfile

#ran on 136/139 (not so sure)
# Test all clause conjoin orders on all EASY (solved by planDD or few variables) opt strips testfiles
def interleaved_easy_test():
    probs = problems.list_all_easy_opt_strips_problems()
    interleaved_arguments = arguments.generate_all_interleaved_argument_maps()
    args = []
    for i in range(len(interleaved_arguments)):
        suite_name = "interleaved_small_" + str(i)
        planDD_argument_map = dict(interleaved_arguments[i])
        downward_argument_map = dict(arguments.standart_downward_argument_map)
        
        planDD_argument_map["$timeout"] = "60s"
        args.append((suite_name, planDD_argument_map, downward_argument_map))
        
    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))

    return comms

# Test all clause conjoin orders on all opt strips testfiles, WARNING! This is very big
def interleaved_all_test():
    probs = problems.list_all_downward_solved_problems()
    interleaved_arguments = arguments.generate_all_interleaved_argument_maps()
    args = []
    for i in range(len(interleaved_arguments)):
        suite_name = "interleaved_BIG_" + str(i)
        planDD_argument_map = dict(interleaved_arguments[i])
        downward_argument_map = dict(arguments.standart_downward_argument_map)
        
        planDD_argument_map["$timeout"] = "60s"
        args.append((suite_name, planDD_argument_map, downward_argument_map))
    
    comms = commandfile.generate_command_calls(probs, args)
    return comms


#ran on 134
def variable_order_no_ladder_test():
    probs = problems.list_all_easy_opt_strips_problems()
    arg_dicts = arguments.generate_all_variable_order_maps()
    args = []
    for i in range(len(arg_dicts)):
        suite_name = "variable_order_no_ladder_" + str(i)
        planDD_argument_map = dict(arg_dicts[i])
        downward_argument_map = dict(arguments.standart_downward_argument_map)
        
        planDD_argument_map["$timeout"] = "60s"
        args.append((suite_name, planDD_argument_map, downward_argument_map))

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms

#ran on 134
def variable_order_with_ladder_test():
    probs = problems.list_all_easy_opt_strips_problems()
    arg_dicts = arguments.generate_all_variable_order_maps_with_ladder_encoding()
    args = []
    for i in range(len(arg_dicts)):
        suite_name = "variable_order_with_ladder_" + str(i)
        planDD_argument_map = dict(arg_dicts[i])
        downward_argument_map = dict(arguments.standart_downward_argument_map)
        
        planDD_argument_map["$timeout"] = "60s"
        args.append((suite_name, planDD_argument_map, downward_argument_map))

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms

# ran on the 139? (the 256 core machine)
def best_17_5_big_test():
    probs = problems.list_all_downward_solved_problems()
    planDD_argument_map =  {
        "$timeout" : "180s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order grtyumix:pec --variable_order x:vohjk --goal_variables_first --initial_state_variables_first ",
    }
    downward_argument_map = dict(arguments.standart_downward_argument_map)
    arg = ("best17_5_big_test", planDD_argument_map, downward_argument_map)
    args = [arg]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


# run on 136 with 32 cores
def best_24_5_big_test():
    probs = problems.list_all_downward_solved_problems()
    planDD_argument_map =  {
        "$timeout" : "180s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order grtyumix:pec --variable_order x:vohjk --goal_variables_first --initial_state_variables_first --exact_one_constraint ",
    }
    downward_argument_map = dict(arguments.standart_downward_argument_map)
    arg = ("best_24_5_big_test", planDD_argument_map, downward_argument_map)
    args = [arg]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms

# 137
def layer_unidirectional_building_test():
    probs = problems.list_all_easy_opt_strips_problems()
    arg_dicts = arguments.generate_layer_building_unidirectional_argument_maps()
    args = []
    for i in range(len(arg_dicts)):
        suite_name = "layer_unidirectional_" + str(i)
        planDD_argument_map = dict(arg_dicts[i])
        downward_argument_map = dict(arguments.standart_downward_argument_map)
        
        planDD_argument_map["$timeout"] = "120s"
        args.append((suite_name, planDD_argument_map, downward_argument_map))

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms

# 137
def layer_bidirectional_building_test():
    probs = problems.list_all_easy_opt_strips_problems()
    arg_dicts = arguments.generate_layer_building_bidirectional_argument_maps()
    args = []
    for i in range(len(arg_dicts)):
        suite_name = "layer_bidirectional_" + str(i)
        planDD_argument_map = dict(arg_dicts[i])
        downward_argument_map = dict(arguments.standart_downward_argument_map)
        
        planDD_argument_map["$timeout"] = "120s"
        args.append((suite_name, planDD_argument_map, downward_argument_map))

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


def best_triple_21_6_big_test():
    probs = problems.list_all_downward_solved_problems()
    planDD_argument_map_old_best =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order grtyumix:pec --variable_order x:vohjk --goal_variables_first --initial_state_variables_first --exact_one_constraint ",
    }
    planDD_argument_map_layer_unidirectional =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd_by_layer",
        "$addition_flags" : " --build_order ig:rtyumpec: --exact_one_constraint --layer_on_the_fly ",
    }
    planDD_argument_map_layer_bidirectional =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd_by_layer",
        "$addition_flags" : " --build_order ig:rtyumpec:ig --exact_one_constraint --bidirectional --layer_on_the_fly --use_layer_permutation ",
    }
    downward_argument_map = dict(arguments.standart_downward_argument_map)
    arg1 = ("best_triple_21_6_big_test_old", planDD_argument_map_old_best, downward_argument_map)
    arg2 = ("best_triple_21_6_big_test_uni", planDD_argument_map_layer_unidirectional, downward_argument_map)
    arg3 = ("best_triple_21_6_big_test_bi", planDD_argument_map_layer_bidirectional, downward_argument_map)
    args = [arg1, arg2, arg3]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms



# TODO test all var orders with all conjoin orders


comms = []
comms += best_triple_21_6_big_test()

commandfile.generate_parallel_file_from_calls(comms)

