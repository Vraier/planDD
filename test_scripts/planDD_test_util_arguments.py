import itertools
from itertools import chain, combinations

# example structure for an argument map
standart_planDD_argument_map = {
    "$timeout" : "60s",
    "$mode" : "build_bdd",
    "$addition_flags" : "",
}

standart_downward_argument_map = {
    "$downward_timeout" : "60s",
    "$problem_path" : "prob.pddl"
}

def generate_layer_building_unidirectional_argument_maps():
    all_dics = []
    all_conjoin_orders = generate_all_layer_orders_unidirectional()
    flags = generate_all_layer_unidirectional_flags()
    for order in all_conjoin_orders:
        for flg in flags:
            new_p_dic = dict(standart_planDD_argument_map)
            new_p_dic["$timeout"] = "120s"
            new_p_dic["$addition_flags"] += (" --build_order " + order) + " --exact_one_constraint " + flg   
            new_p_dic["$mode"] = "build_bdd_by_layer"
            all_dics.append(new_p_dic)    
    return all_dics

# TODO use best flags of unidirectional here
def generate_layer_building_bidirectional_argument_maps():
    all_dics = []
    all_conjoin_orders = generate_all_layer_orders_bidirectional()
    flags = generate_all_layer_bidirectional_flags()
    for order in all_conjoin_orders:
        for flg in flags:
            new_p_dic = dict(standart_planDD_argument_map)
            new_p_dic["$timeout"] = "120s"
            new_p_dic["$addition_flags"] += (" --build_order " + order)
            new_p_dic["$addition_flags"] += " --exact_one_constraint --bidirectional "
            new_p_dic["$addition_flags"] += " " + flg   
            new_p_dic["$mode"] = "build_bdd_by_layer"
            all_dics.append(new_p_dic)    
    return all_dics

def generate_all_interleaved_argument_maps():
    all_dics = []
    all_interleaved_orders = generate_all_interleaved_clause_orders()
    for order in all_interleaved_orders:
        new_p_dic = dict(standart_planDD_argument_map)
        new_p_dic["$addition_flags"] += (" --build_order " + order)    
        all_dics.append(new_p_dic)    
    return all_dics

def generate_all_variable_order_maps():
    all_dics = []
    no_ladder_encoding_orders = generate_variables_orders_without_helper_variables()
    for order in no_ladder_encoding_orders:
        for goal_first in True,False:
            for inital_state_frist in True,False:
                for include_mutex in True,False:
                    new_p_dic = dict(standart_planDD_argument_map)
                    new_p_dic["$addition_flags"] += ("--variable_order " + order)   
                    if(goal_first):
                        new_p_dic["$addition_flags"] += " --goal_variables_first"
                    if(inital_state_frist):
                        new_p_dic["$addition_flags"] += " --initial_state_variables_first"
                    if(include_mutex):
                        new_p_dic["$addition_flags"] += " --include_mutex"
                    all_dics.append(new_p_dic)    
    return all_dics

def generate_all_variable_order_maps_with_ladder_encoding():
    all_dics = []
    ladder_encoding_orders = generate_variable_orders_with_herlper_variables()
    for order in ladder_encoding_orders:
        for goal_first in True,False:
            for inital_state_frist in True,False:
                for include_mutex in True,False:
                    new_p_dic = dict(standart_planDD_argument_map)
                    new_p_dic["$addition_flags"] += ("--variable_order " + order)   
                    new_p_dic["$addition_flags"] += " --use_ladder_encoding"  
                    if(goal_first):
                        new_p_dic["$addition_flags"] += " --goal_variables_first"
                    if(inital_state_frist):
                        new_p_dic["$addition_flags"] += " --initial_state_variables_first"
                    if(include_mutex):
                        new_p_dic["$addition_flags"] += " --include_mutex"
                    all_dics.append(new_p_dic)    
    return all_dics


########################################################################################
# Helper methods that generate sets of arguments

def generate_all_layer_bidirectional_flags():
    result = ["", "--share_foundations"]
    return result

def generate_all_layer_unidirectional_flags():
    result = []
    possible_flags = ["--use_layer_permutation", "--layer_on_the_fly"]
    powerset = list(chain.from_iterable(combinations(possible_flags, r) for r in range(len(possible_flags)+1)))
    for subset in powerset:
        result.append(" ".join(list(subset)))
    return result
    
def generate_all_layer_orders_unidirectional():
    ini_foundations = ["ig", "igrtyum", "rtyumig"]
    layers = ["rtyum" + "".join(p) for p in itertools.permutations(["p", "e", "c"])]
    result = []
    for ini in ini_foundations:
        for layer in layers:
            result.append(ini + ":" + layer + ":")
    return result

def generate_all_layer_orders_bidirectional():
    ini_foundations = ["i", "irtyum", "rtyumi", "ig"]
    goal_foundations = ["g", "grtyum", "rtyumg", "ig"]
    layers = ["rtyum" + "".join(p) for p in itertools.permutations(["p", "e", "c"])]
    result = []
    for ini in ini_foundations:
        for goal in goal_foundations:
            for layer in layers:
                result.append(ini + ":" + layer + ":" + goal)
    return result

# Generates only disjoint orders
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

# also uses the interleaved part
def generate_all_interleaved_clause_orders():
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
    # choose which parts not to interleave but order disjoint
    # (interleaving only one or zero part(s) does are edge cases, i dont do interleaved parts of size 0)
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
def reverse_order(order_string):
    s1, s2 = order_string.split(":")[0], order_string.split(":")[1]
    return s1[::-1] + ":" + s2[::-1]


def generate_variables_orders_without_helper_variables():
    return ["vox:hjk", "ovx:hjk", "x:vohjk", "x:ovhjk"]

# in case of using ladder encoding
def generate_variable_orders_with_herlper_variables():
    all_orders = []
    tags = [
        "v",
        "o",
        "hjk",
    ]
    powerset = list(chain.from_iterable(combinations(tags, r) for r in range(2, len(tags)+1)))

    for subset in powerset:
        local_interleaved = list(subset)
        local_disjoint = [x for x in tags if x not in subset] + ["x"]

        interleaved_permutations = list(itertools.permutations(local_interleaved))
        disjoint_permutations = list(itertools.permutations(local_disjoint))

        for s in interleaved_permutations:
            for p in disjoint_permutations:
                new_order = "".join(p) + ":" + "".join(s)
                all_orders.append(new_order)
    
    return all_orders


#print(len(generate_layer_building_unidirectional_argument_maps()))