import planDD_test_util_problems as problems

import random
import os

def draw_random_problem():
    while True:
        rand_prob = random.choice(problems.list_all_easy_opt_strips_problems())
        #if(not "gripper" in rand_prob['path']):
        #    continue
        template_downward = "../../downward/fast-downward.py --sas-file prob.sas --translate $path"
        template_planDD = "../build/planDD --sas_file prob.sas --conflict_graph"
        template_graphviz = "neato -Tpng graph.dot -o graph.png"
        
        downward_command = template_downward.replace("$path", rand_prob['path'], 1)
        os.system(downward_command)
        os.system(template_planDD)
        print("Drawing", rand_prob['path'])
        os.system(template_graphviz)
        print("Domain, Problem:",rand_prob["d_name"], rand_prob["p_name"])
        break
    
    
draw_random_problem()