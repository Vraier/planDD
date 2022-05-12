import itertools
from itertools import chain, combinations

# example structure for an argument map
standart_planDD_argument_map = {
    "$timeout" : "60s",
    "$mode" : "build_bdd",
    "$timesteps" : "13",
    "$build_order" : "igrtyumpecx:",
    "$addition_flags" : "",
}

standart_downward_argument_map = {
    "$downward_timeout" : "60s",
    "$problem_path" : "prob.pddl"
}

def generate_all_interleaved_argument_maps():
    all_dics = []
    all_interleaved_orders = generate_all_interleaved_clause_orders()
    for order in all_interleaved_orders:
        new_p_dic = dict(standart_planDD_argument_map)
        new_p_dic["$build_order"] = order    
        all_dics.append(new_p_dic)    
    return all_dics
    


# Helper methods that generate sets of arguments

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