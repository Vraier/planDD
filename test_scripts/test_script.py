#all_argument_maps = generate_argmument_maps_for_conjoin_orders(standart_argument_map, generate_all_interleaved_orders())
#all_commands = generate_all_commandline_strings(standart_commandline_string, all_argument_maps)

# Test Run for including mutex constraints
"""
best_orders = [plot_data.unsimplify_order_string(x) for x in plot_data.find_all_finished_orders("../test_output/interleaved_bdd")]
mutex_standart_map = standart_argument_map.copy()
mutex_standart_map["$addition_flags"] = "--include_mutex"
mutex_standart_map["$output_folder"] = "../test_output/include_mutex"
mutex_arguments_maps = generate_argmument_maps_for_conjoin_orders(mutex_standart_map, best_orders)
all_commands = generate_all_commandline_strings(standart_commandline_string, mutex_arguments_maps)
"""

# Test Run for trying to reverse the best orders
"""
best_orders = [plot_data.unsimplify_order_string(x) for x in plot_data.find_all_finished_orders("../test_output/interleaved_bdd")]
reversed_best_orders = [reverse_order(x) for x in best_orders]
reverse_standart_map = standart_argument_map.copy()
reverse_standart_map["$addition_flags"] = "--reverse_order"
reverse_standart_map["$output_folder"] = "../test_output/reverse_order"
reverse_argument_maps = generate_argmument_maps_for_conjoin_orders(reverse_standart_map, reversed_best_orders)
all_commands = generate_all_commandline_strings(standart_commandline_string, reverse_argument_maps)
"""

# test all 
all_interleaved_orders = generate_all_interleaved_clause_orders()


#print(all_commands)
#execute_all_commands(all_commands)

print(generate_all_interleaved_clause_orders())
print(len(generate_all_interleaved_clause_orders()))