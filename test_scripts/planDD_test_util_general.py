def get_sanitized_domain_description(domain_name, problem_name):
    return "".join(x for x in domain_name+problem_name if x.isalnum())