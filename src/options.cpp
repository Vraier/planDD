#include "options.h"

namespace po = boost::program_options;

void option_parser::parse_command_line(int argc, char *argv[]) {
    m_option_desc.add_options()("help", "produce help message")
        // Filepaths
        ("sas_file", po::value<std::string>(&m_values.sas_file)->default_value("output.sas"),
         "Input sas file, describing the planning problem")  //
        ("ass_file", po::value<std::string>(&m_values.ass_file)->default_value("minisat.ass"),
         "Input cnf file, containing a solution to a cnf problem")  //
        ("cnf_file", po::value<std::string>(&m_values.cnf_file)->default_value("problem.cnf"),
         "A cnf file describing the planning problem")  //
        // Program modes
        ("encode_cnf", po::bool_switch(&m_values.encode_cnf)->default_value(false),
         "encodes the planning problem into a cnf writes it to a file")  //
        ("cnf_to_bdd", po::bool_switch(&m_values.cnf_to_bdd)->default_value(false),
         "Builds a bdd from a cnf file")  //
        ("conflict_graph", po::bool_switch(&m_values.conflict_graph)->default_value(false),
         "Creates the action conflic graph for the given sas problem and wirtes it to a file.")  //
        ("build_bdd", po::bool_switch(&m_values.build_bdd)->default_value(false),
         "tries to build the bdd for the given planning problem")  //
        ("build_sdd", po::bool_switch(&m_values.build_sdd)->default_value(false),
         "tries to build the sdd for the given planning problem")  //
        ("single_minisat", po::bool_switch(&m_values.single_minisat)->default_value(false),
         "converts the given cnf to a solution for the planning problem")  //
        ("count_minisat", po::bool_switch(&m_values.count_minisat)->default_value(false),
         "Runs minisat to solve a cnf problem generated by this program")  //
        ("hack_debug", po::bool_switch(&m_values.hack_debug)->default_value(false),
         "Sandbox hack mode for developing purposes")  //
        // DD building parameters
        ("timesteps", po::value<int>(&m_values.timesteps)->default_value(-1),
         "The amount of timsteps represented by the cnf formula")  //
        ("num_plans", po::value<double>(&m_values.num_plans)->default_value(-1.0),
         "The amount of plans the planner will search for")  //
        // what and how to conjoin clauses?
        ("include_mutex", po::bool_switch(&m_values.include_mutex)->default_value(false),
         "If this flag is set, the cnf encoder will include the mutexes from the sas problem in its formula")  //
        ("use_ladder_encoding", po::bool_switch(&m_values.use_ladder_encoding)->default_value(false),
         "Uses the ladder encoding for at most one constraints")  //
        ("group_pre_eff", po::bool_switch(&m_values.group_pre_eff)->default_value(false),
         "Uses DNF primitives to group the precondition and effect constraints into a singel DNF")  //
        ("exact_one_constraint", po::bool_switch(&m_values.exact_one_constraint)->default_value(false),
         "Builds the exactly one variable is true constraints directly into the DD")  //
        ("parallel_plan", po::bool_switch(&m_values.parallel_plan)->default_value(false),
         "Only prohibits conflicting operators in one timestep. Does not work with exact one encoding or binary "
         "encoding.")  //
        ("binary_encoding", po::bool_switch(&m_values.binary_encoding)->default_value(false),
         "Encodes the actions in a binary and not a unary way. Resulting in only log(|action|) variables.")  //
        ("binary_variables", po::bool_switch(&m_values.binary_variables)->default_value(false),
         "Encodes the mutiple values of a planning variable in a binary and not a unary way. Resulting in only "
         "log(|value|) variables.")  //
        ("binary_exclude_impossible", po::bool_switch(&m_values.binary_exclude_impossible)->default_value(false),
         "Excludes the impossible (dummy) actions from the logic formula, if binary encoding is used.")  //
        ("binary_parallel", po::bool_switch(&m_values.binary_parallel)->default_value(false),
         "Combination of binary and parallel plan encoding. Should be used wit timestep -1.")  //
        ("build_order", po::value<std::string>(&m_values.build_order)->default_value("igx:rympec:"),
         "Determins the order of conjoins when building a dd linearily and not interleaved. Must be a permutation of "
         "the string impgc; i: initial_state, rtyum: mutex, pe: precondition/effect, g: goal, c: changing atoms "
         "implication.")  //
        ("reverse_order", po::bool_switch(&m_values.reverse_order)->default_value(false),
         "Reverses the order of the conjoin operations. This has the effect that the conjoin order gets reversed but "
         "also clauses with higher timesteps get conjoined first.")  //
        // variable ordering
        ("no_reordering", po::bool_switch(&m_values.no_reordering)->default_value(false),
         "Disables automatic reordering during dd construction.")  //
        ("variable_order", po::value<std::string>(&m_values.variable_order)->default_value("x:voh"),
         "Determins the initial variable order for the dd building. "
         "v: variables, o: operators, h: helper amost variable, j: helper amost operator, k: helper amost mutex")  //
        ("goal_variables_first", po::bool_switch(&m_values.goal_variables_first)->default_value(false),
         "If this flag is set, variables in goal clauses will be moved to the front of the variable order")  //
        ("initial_state_variables_first",
         po::bool_switch(&m_values.initial_state_variables_first)->default_value(false),
         "If this flag is set, variables in initial state clauses will be moved to the front of the variable order")  //
        ("var_order_force", po::bool_switch(&m_values.var_order_force)->default_value(false),
         "Use force to order all variables")  //
        ("var_order_custom", po::bool_switch(&m_values.var_order_custom)->default_value(false),
         "Use custom to order all variables (needs variable_order string)")  //
        ("var_order_custom_force", po::bool_switch(&m_values.var_order_custom_force)->default_value(false),
         "Combines force and custom")  //
        ("clause_order_force", po::bool_switch(&m_values.clause_order_force)->default_value(false),
         "Use force to order all clauses")  //
        ("clause_order_custom", po::bool_switch(&m_values.clause_order_custom)->default_value(false),
         "Use custom to order all clauses (needs variable_order string)")  //
        ("clause_order_custom_force", po::bool_switch(&m_values.clause_order_custom_force)->default_value(false),
         "Combines force and custom")  //
        ("clause_order_bottom_up", po::bool_switch(&m_values.clause_order_bottom_up)->default_value(false),
         "Orders all variables after the bottom up heuristic. It uses the set variable order. Only applies bottom up "
         "once")  //
        ("clause_order_custom_bottom_up",
         po::bool_switch(&m_values.clause_order_custom_bottom_up)->default_value(false),
         "uses custom order and bottom up as tiebreaker")  //
        ("split_inside_timestep", po::bool_switch(&m_values.split_inside_timestep)->default_value(false),
         "If set, the tiebreaker for mixed ordering is only applied for each category inside a timestep. If not setp, "
         "the tiebreaker is used for the whole timesetp")  //
        ("force_random_seed", po::bool_switch(&m_values.force_random_seed)->default_value(false),
         "Sets the initial order for the force algorithm to a random permutation and does not use custom order")  //
        // variable grouping
        ("group_variables", po::bool_switch(&m_values.group_variables)->default_value(false),
         "Groups the variables for one timestep together")  //
        ("group_variables_small", po::bool_switch(&m_values.group_variables_small)->default_value(false),
         "Groups the variables for one timestep together. It only groups the variables for a singel planning variables "
         "(and not all planning variables of a timestep)")  //
        ("group_actions", po::bool_switch(&m_values.group_actions)->default_value(false),
         "Groups the actions for one timestep together. Works with unary and binary encoding")  //
        // building algorithm
        ("linear", po::bool_switch(&m_values.linear)->default_value(false),
         "Builds the dd clause by clause")  //
        ("layer", po::bool_switch(&m_values.layer)->default_value(false),
         "tries to build the bdd for the given planning problem layer by layer")  //
        ("layer_bi", po::bool_switch(&m_values.layer_bi)->default_value(false),
         "If this flag is set, the the bdd layer construction will work in a bidriectional manner")  //
        ("layer_expo", po::bool_switch(&m_values.layer_expo)->default_value(false),
         "If this flag is set, the layers will be constructed exponentially.")  //
        ("share_foundations", po::bool_switch(&m_values.share_foundations)->default_value(false),
         "If this flag is set during bidirectional lyer construction, both start nodes of the bidirectional search "
         "will be the same")  //
        ("reverse_layer_building", po::bool_switch(&m_values.reverse_layer_building)->default_value(false),
         "If this flag is set, the layer bdds will be constructed, beginning from the goal, from last timestep to "
         "first")  //
        ("use_layer_permutation", po::bool_switch(&m_values.use_layer_permutation)->default_value(false),
         "If this flag is set, the new layer bdd will be constructed using a variable permutation");  //

    po::store(po::parse_command_line(argc, argv, m_option_desc), m_argument_map);
    po::notify(m_argument_map);
}

bool option_parser::check_validity() {
    if ((m_values.encode_cnf + m_values.build_bdd + m_values.build_sdd + m_values.single_minisat +
         m_values.count_minisat + m_values.hack_debug + m_values.cnf_to_bdd + m_values.conflict_graph) != 1) {
        std::cout << "You have to choose exactly one mode." << std::endl;
        return false;
    }
    return true;
}

void option_parser::print_help() {
    std::cout << "Currently the program can encode a planning problem into a cnf, build a bdd from it or decode a "
                 "cnf solution to a plan. You should only choose one of the options at a time."
              << std::endl;
    std::cout << m_option_desc << std::endl;
}

// code from (little bit modified): https://gist.github.com/gesquive/8673796
void option_parser::print_variable_map() {
    std::cout << "Using the following config: " << std::endl;

    for (po::variables_map::const_iterator it = m_argument_map.begin(); it != m_argument_map.end(); it++) {
        std::cout << "> " << it->first;
        if (((boost::any)it->second.value()).empty()) {
            std::cout << "(empty)";
        }
        if (m_argument_map[it->first].defaulted() || it->second.defaulted()) {
            std::cout << "(default)";
        }
        std::cout << "=";

        bool is_char;
        try {
            boost::any_cast<const char *>(it->second.value());
            is_char = true;
        } catch (const boost::bad_any_cast &) {
            is_char = false;
        }
        bool is_str;
        try {
            boost::any_cast<std::string>(it->second.value());
            is_str = true;
        } catch (const boost::bad_any_cast &) {
            is_str = false;
        }

        if (((boost::any)it->second.value()).type() == typeid(int)) {
            std::cout << m_argument_map[it->first].as<int>() << std::endl;
        } else if (((boost::any)it->second.value()).type() == typeid(bool)) {
            std::cout << (m_argument_map[it->first].as<bool>() ? "true" : "false") << std::endl;
        } else if (((boost::any)it->second.value()).type() == typeid(double)) {
            std::cout << m_argument_map[it->first].as<double>() << std::endl;
        } else if (is_char) {
            std::cout << m_argument_map[it->first].as<const char *>() << std::endl;
        } else if (is_str) {
            std::string temp = m_argument_map[it->first].as<std::string>();
            if (temp.size()) {
                std::cout << temp << std::endl;
            } else {
                std::cout << "true" << std::endl;
            }
        } else {  // Assumes that the only remainder is vector<string>
            try {
                std::vector<std::string> vect = m_argument_map[it->first].as<std::vector<std::string> >();
                uint i = 0;
                for (std::vector<std::string>::iterator oit = vect.begin(); oit != vect.end(); oit++, ++i) {
                    std::cout << "\r> " << it->first << "[" << i << "]=" << (*oit) << std::endl;
                }
            } catch (const boost::bad_any_cast &) {
                std::cout << "UnknownType(" << ((boost::any)it->second.value()).type().name() << ")" << std::endl;
            }
        }
    }
}