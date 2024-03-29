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


#137?
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

#137
def best_triple_28_6_big_test():
    probs = problems.list_all_downward_solved_problems()
    planDD_argument_map_old_best =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order grtyumix:pec --variable_order x:vohjk --goal_variables_first --initial_state_variables_first --exact_one_constraint ",
    }
    planDD_argument_map_layer_reverse_no_perm =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd_by_layer",
        "$addition_flags" : " --build_order ig:rtyumpec: --exact_one_constraint --reverse_layer_building ",
    }
    planDD_argument_map_layer_reverse_with_perm =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd_by_layer",
        "$addition_flags" : " --build_order ig:rtyumpec: --exact_one_constraint --reverse_layer_building --use_layer_permutation ",
    }
    planDD_argument_map_layer_exponential =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd_by_layer",
        "$addition_flags" : " --build_order ig:rtyumpec: --exact_one_constraint --exponential ",
    }
    downward_argument_map = dict(arguments.standart_downward_argument_map)
    arg1 = ("best_triple_28_6_big_test_old", planDD_argument_map_old_best, downward_argument_map)
    arg2 = ("best_triple_28_6_big_test_reverse_no_perm", planDD_argument_map_layer_reverse_no_perm, downward_argument_map)
    arg3 = ("best_triple_28_6_big_test_reverse_with_perm", planDD_argument_map_layer_reverse_with_perm, downward_argument_map)
    arg4 = ("best_triple_28_6_big_test_expo", planDD_argument_map_layer_exponential, downward_argument_map)
    args = [arg1, arg2, arg3, arg4]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


#137
def best_13_7_big_test():
    probs = problems.list_all_downward_solved_problems()
    planDD_argument_map_old_best =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order igx:rympec: --exact_one_constraint ",
    }
    planDD_argument_map_no_timesteps =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order rympec:: --exact_one_constraint ",
    }
    planDD_argument_map_no_timesteps_parallel =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order rympec:: --exact_one_constraint --parallel_plan ",
    }
    #TODO: this seems to be very sensitive to conjoin order. thurther testing needed
    # I dodged a bullet here. nealry threw this approach away
    planDD_argument_map_binary_encoding =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --build_order igx:rympec: --exact_one_constraint --binary_encoding ",
    }
    downward_argument_map = dict(arguments.standart_downward_argument_map)
    arg1 = ("best_13_7_big_test_old", planDD_argument_map_old_best, downward_argument_map)
    arg2 = ("best_13_7_big_test_no_timesteps", planDD_argument_map_no_timesteps, downward_argument_map)
    arg3 = ("best_13_7_big_test_no_timesteps_parallel", planDD_argument_map_no_timesteps_parallel, downward_argument_map)
    arg4 = ("best_13_7_big_test_binary_encoding", planDD_argument_map_binary_encoding, downward_argument_map)
    args = [arg1, arg2, arg3, arg4]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


#135
def best_20_7_big_test():
    probs = problems.list_all_downward_solved_problems()
    map_old_best =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --exact_one_constraint ",
    }

    map_no_t =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec:: --exact_one_constraint ",
    }
    map_no_t_binary =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec:: --binary_encoding ",
    }
    map_no_t_no_reorder =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec:: --exact_one_constraint --no_reordering ",
    }
    map_no_t_no_reorder_binary =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec::  --no_reordering --binary_encoding ",
    }

    map_group_action =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --exact_one_constraint --group_actions ",
    }
    map_group_var =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --exact_one_constraint --group_variables ",
    }
    map_group_varsmall =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --exact_one_constraint --group_variables_small ",
    }
    map_group_var_action =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --exact_one_constraint --group_actions --group_variables ",
    }
    map_group_varsmall_action =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --exact_one_constraint --group_actions --group_variables_small ",
    }

    map_group_action_binary =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --group_actions --binary_encoding ",
    }
    map_group_var_binary =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --group_variables --binary_encoding ",
    }
    map_group_varsmall_binary =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --group_variables_small --binary_encoding ",
    }
    map_group_var_action_binary =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --group_actions --group_variables --binary_encoding ",
    }
    map_group_varsmall_action_binary =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --group_actions --group_variables_small --binary_encoding ",
    }

    map_binary_no_reorder =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --binary_encoding --no_reordering ",
    }

    downward_argument_map = dict(arguments.standart_downward_argument_map)

    arg1 = ("best_20_7_old_best", map_old_best, downward_argument_map)
    arg2 = ("best_20_7_no_t", map_no_t, downward_argument_map)
    arg3 = ("best_20_7_no_t_binary", map_no_t_binary, downward_argument_map)
    arg4 = ("best_20_7_no_t_no_reorder", map_no_t_no_reorder, downward_argument_map)
    arg5 = ("best_20_7_no_t_no_reorder_binary", map_no_t_no_reorder_binary, downward_argument_map)
    arg6 = ("best_20_7_group_action", map_group_action, downward_argument_map)
    arg7 = ("best_20_7_group_var", map_group_var, downward_argument_map)
    arg8 = ("best_20_7_group_varsmall", map_group_varsmall, downward_argument_map)
    arg9 = ("best_20_7_group_var_action", map_group_var_action, downward_argument_map)
    arg10 = ("best_20_7_group_varsmall_action", map_group_varsmall_action, downward_argument_map)
    arg11 = ("best_20_7_group_action_binary", map_group_action_binary, downward_argument_map)
    arg12 = ("best_20_7_group_var_binary", map_group_var_binary, downward_argument_map)
    arg13 = ("best_20_7_group_varsmall_binary", map_group_varsmall_binary, downward_argument_map)
    arg14 = ("best_20_7_group_var_action_binary", map_group_var_action_binary, downward_argument_map)
    arg15 = ("best_20_7_group_varsmall_action_binary", map_group_varsmall_action_binary, downward_argument_map)
    arg16 = ("best_20_7_binary_no_reorder", map_binary_no_reorder, downward_argument_map)
    args = [
        arg1, 
        arg3, 
        arg16,
        arg5, 

        arg11,
        arg12,
        arg13,
        arg14,
        arg15,

        arg2, 
        arg4,

        arg6, 
        arg7, 
        arg8, 
        arg9, 
        arg10,
    ]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


#137/138? had 240 cores!
def best_27_7_big_test():
    probs = problems.list_all_downward_solved_problems()
    downward_argument_map = dict(arguments.standart_downward_argument_map)

    map_old_best =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --no_reordering --binary_encoding ",
    }
    map_binary_var =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --no_reordering --binary_encoding --binary_variables ",
    }
    map_binary_no_imp =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --no_reordering --binary_encoding --binary_exclude_impossible ",
    }
    map_binary_var_no_imp =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --no_reordering --binary_encoding --binary_variables --binary_exclude_impossible ",
    }
    map_binary_reorder =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --binary_encoding ",
    }
    map_binary_var_reorder =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --binary_encoding --binary_variables ",
    }
    map_binary_no_imp_reorder =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --binary_encoding --binary_exclude_impossible ",
    }
    map_binary_var_no_imp_reorder =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --binary_encoding --binary_variables --binary_exclude_impossible ",
    }

    arg1 = ("best_27_7_old_best", map_old_best, downward_argument_map)
    arg2 = ("best_27_7_binary_var", map_binary_var, downward_argument_map)
    arg3 = ("best_27_7_binary_no_imp", map_binary_no_imp, downward_argument_map)
    arg4 = ("best_27_7_binary_var_no_imp", map_binary_var_no_imp, downward_argument_map)
    arg5 = ("best_27_7_binary_reorder", map_binary_reorder, downward_argument_map)
    arg6 = ("best_27_7_binary_var_reorder", map_binary_var_reorder, downward_argument_map)
    arg7 = ("best_27_7_binary_no_imp_reorder", map_binary_no_imp_reorder, downward_argument_map)
    arg8 = ("best_27_7_binary_var_no_imp_reorder", map_binary_var_no_imp_reorder, downward_argument_map)

    args = [
        arg1, 
        arg2, 
        arg3, 
        arg4, 
        arg5,
        arg6, 
        arg7, 
        arg8,  
    ]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms

#134, old friend
def best_09_08_big_test():
    probs = problems.list_all_downward_solved_problems()
    downward_argument_map = dict(arguments.standart_downward_argument_map)

    # best_27_7_binary_var_no_imp_reorder
    map_old_best =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --binary_encoding --binary_variables --binary_exclude_impossible ",
    }
    map_binary_parallel = {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec:: --binary_parallel ",
    }
    map_unary_parallel = {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec:: --parallel_plan ",
    }
    map_unary_incremental = {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec:: ",
    }
    map_binary_incremental = {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order rympec:: --binary_encoding --binary_variables --binary_exclude_impossible ",
    }


    map_group_pre_eff =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --binary_encoding --binary_variables --binary_exclude_impossible --group_pre_eff",
    }


    arg1 = ("best_09_08_old_best", map_old_best, downward_argument_map)
    arg2 = ("best_09_08_binary_parallel", map_binary_parallel, downward_argument_map)
    arg3 = ("best_09_08_unary_parallel", map_unary_parallel, downward_argument_map)
    arg4 = ("best_09_08_unary_incremental", map_unary_incremental, downward_argument_map)
    arg5 = ("best_09_08_binary_incremental", map_binary_incremental, downward_argument_map)
    arg6 = ("best_09_08_group_pre_eff", map_group_pre_eff, downward_argument_map)


    args = [
        arg1, 
        arg2, 
        arg3, 
        arg4, 
        arg5,
        arg6,  
    ]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


def gen_args_for_03_08():
    args = []

    binary_encode_args = "--binary_encoding --binary_variables --binary_exclude_impossible"
    downward_argument_map = dict(arguments.standart_downward_argument_map)
    clause_orders = [   
        "clause_order_custom",
        "clause_order_force",
        "clause_order_custom_force",
        "clause_order_bottom_up",
        "clause_order_custom_bottom_up",
    ]
    map_template = {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --variable_order x:voh --var_order_custom",
    }

    # with binary encoding
    for order in clause_orders:
        new_arg_map = dict(map_template)
        new_arg_map["$addition_flags"] += " --" + order + " " + binary_encode_args

        if "force" in order:
            add_desc = "03_08_" + order + "_binary_rand_seed"
            add_map = dict(new_arg_map)
            add_map["$addition_flags"] += " --force_random_seed"
            args.append((add_desc, add_map, downward_argument_map))

        descriptor = "03_08_" + order + "_binary"
        args.append((descriptor, new_arg_map, downward_argument_map))

    # without binary encoding
    for order in clause_orders:
        new_arg_map = dict(map_template)
        new_arg_map["$addition_flags"] += " --" + order + " --exact_one_constraint"

        if "force" in order:
            add_desc = "03_08_" + order + "_unary_rand_seed"
            add_map = dict(new_arg_map)
            add_map["$addition_flags"] += " --force_random_seed"
            args.append((add_desc, add_map, downward_argument_map))

        descriptor = "03_08_" + order + "_unary"
        args.append((descriptor, new_arg_map, downward_argument_map))

    return args

# first half 137, second 136
def test_03_08_clause_ordering():
    probs = problems.list_all_downward_solved_problems()
   
    args = gen_args_for_03_08()[7:] #WARNING only second half is used here


    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


def gen_args_for_04_08():
    args = []

    binary_encode_args = "--binary_encoding --binary_variables --binary_exclude_impossible"
    downward_argument_map = dict(arguments.standart_downward_argument_map)
    var_orders = [   
        "var_order_custom",
        "var_order_force",
        "var_order_custom_force",
    ]
    map_template = {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : " --linear --build_order igx:rympec: --variable_order x:voh --clause_order_custom",
    }
    
    # without reordering
    for order in var_orders:
        new_arg_map = dict(map_template)
        new_arg_map["$addition_flags"] += " --" + order + " " + binary_encode_args + " --no_reordering"

        if "force" in order:
            add_desc = "04_08_" + order + "_no_reorder_rand_seed"
            add_map = dict(new_arg_map)
            add_map["$addition_flags"] += " --force_random_seed"
            args.append((add_desc, add_map, downward_argument_map))

        descriptor = "04_08_" + order + "_no_reorder"
        args.append((descriptor, new_arg_map, downward_argument_map))

    # with reordering
    for order in var_orders:
        new_arg_map = dict(map_template)
        new_arg_map["$addition_flags"] += " --" + order + " " + binary_encode_args

        if "force" in order:
            add_desc = "04_08_" + order + "_reorder_rand_seed"
            add_map = dict(new_arg_map)
            add_map["$addition_flags"] += " --force_random_seed"
            args.append((add_desc, add_map, downward_argument_map))

        descriptor = "04_08_" + order + "_reorder"
        args.append((descriptor, new_arg_map, downward_argument_map))

    return args

#136,
def test_04_08_clause_ordering():
    probs = problems.list_all_downward_solved_problems()
   
    args = gen_args_for_04_08()
    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms

# TODO test binary encoding with different clause/var orerings
# TODO test all var orders with all conjoin orders

#
def test_21_11_restarting_sdd():
    probs = problems.list_all_downward_solved_problems()
    downward_argument_map = dict(arguments.standart_downward_argument_map)

    map_old_best =  {
        "$timeout" : "300s",
        "$mode" : "build_bdd",
        "$addition_flags" : "--linear --build_order igx:rympec: --variable_order x:voh --clause_order_custom --var_order_custom --binary_encoding --binary_variables --binary_exclude_impossible",
    }
    map_sdd = {
        "$timeout" : "300s",
        "$mode" : "build_sdd",
        "$addition_flags" : "--linear --build_order igx:rympec: --clause_order_custom --binary_encoding --binary_variables --binary_exclude_impossible",
    }

    map_incremental =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : "--linear --build_order rympec:: --variable_order x:voh --clause_order_custom --var_order_custom --binary_encoding --binary_variables --binary_exclude_impossible --num_plans 10000000",
    }
    map_incremental_restart =  {
        "$timeout" : "300s",
        "$timesteps" : -1,
        "$mode" : "build_bdd",
        "$addition_flags" : "--linear --build_order igx:rympec: --variable_order x:voh --clause_order_custom --var_order_custom --binary_encoding --binary_variables --binary_exclude_impossible --num_plans 10000000 --restart",
    }

    arg1 = ("best_21_11_old_best", map_old_best, downward_argument_map)
    arg2 = ("best_21_11_sdd", map_sdd, downward_argument_map)
    arg3 = ("best_21_11_incremental", map_incremental, downward_argument_map)
    arg4 = ("best_21_11_incremental_restart", map_incremental_restart, downward_argument_map)
    args = [
        arg1, 
        arg2, 
        arg3, 
        arg4,  
    ]

    comms = commandfile.generate_command_calls(probs, args)
    print("Num problems:",len(probs))
    print("Num configs:",len(args))
    print("Num commands:",len(comms))
    return comms


comms = []
comms += test_21_11_restarting_sdd()


commandfile.generate_parallel_file_from_calls(comms)

