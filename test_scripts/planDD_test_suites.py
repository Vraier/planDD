import planDD_test_util_problems as problems
import planDD_test_util_arguments as arguments
import planDD_test_util_commandfile as commandfile

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
    probs = problems.list_all_opt_strips_problems()
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

comms = []
comms += variable_order_no_ladder_test()
comms += variable_order_with_ladder_test()

commandfile.generate_parallel_file_from_calls(comms)

