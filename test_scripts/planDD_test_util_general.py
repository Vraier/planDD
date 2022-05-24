PATH_TO_BENCHMARKS = "../../downward-benchmarks/"
PATH_TO_DOWNWARD = "../../downward/fast-downward.py"
PATH_TO_PLANDD = "../build/planDD"
PATH_TO_TEST_OUTPUT = "../../test_output"

def get_sanitized_domain_description(domain_name, problem_name):
    return "".join(x for x in domain_name+problem_name if x.isalnum())