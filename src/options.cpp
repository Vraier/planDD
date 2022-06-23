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
        ("build_bdd", po::bool_switch(&m_values.build_bdd)->default_value(false),
         "tries to build the bdd for the given planning problem")  //
        ("build_bdd_by_layer", po::bool_switch(&m_values.build_bdd_by_layer)->default_value(false),
         "tries to build the bdd for the given planning problem layer by layer")  //
        ("build_sdd", po::bool_switch(&m_values.build_sdd)->default_value(false),
         "tries to build the sdd for the given planning problem")  //
        ("single_minisat", po::bool_switch(&m_values.single_minisat)->default_value(false),
         "converts the given cnf to a solution for the planning problem")  //
        ("count_minisat", po::bool_switch(&m_values.count_minisat)->default_value(false),
         "Runs minisat to solve a cnf problem generated by this program")  //
        ("hack_debug", po::bool_switch(&m_values.hack_debug)->default_value(false),
         "Sandbox hack mode for developing purposes")  //
        // DD building parameters
        ("timesteps", po::value<int>(&m_values.timesteps),
         "The amount of timsteps represented by the cnf formula")  //
        // what and how to conjoin clauses?
        ("include_mutex", po::bool_switch(&m_values.include_mutex)->default_value(false),
         "If this flag is set, the cnf encoder will include the mutexes from the sas problem in its formula")  //
        ("use_ladder_encoding", po::bool_switch(&m_values.use_ladder_encoding)->default_value(false),
         "Uses the ladder encoding for at most one constraints")  //
        ("exact_one_constraint", po::bool_switch(&m_values.exact_one_constraint)->default_value(false),
         "Builds the exactly one variable is true constraints directly into the DD")  //
        ("build_order", po::value<std::string>(&m_values.build_order)->default_value("grtyumix:pec"),
         "Determins the order of conjoins when building a dd linearily and not interleaved. Must be a permutation of "
         "the string impgc; i: initial_state, rtyum: mutex, pe: precondition/effect, g: goal, c: changing atoms "
         "implication.")  //
        ("reverse_order", po::bool_switch(&m_values.reverse_order)->default_value(false),
         "Reverses the order of the conjoin operations. This has the effect that the conjoin order gets reversed but "
         "also clauses with higher timesteps get conjoined first.")  //
        // variable ordering
        ("variable_order", po::value<std::string>(&m_values.variable_order)->default_value("x:vohjk"),
         "Determins the initial variable order for the dd building. "
         "v: variables, o: operators, h: helper amost variable, j: helper amost operator, k: helper amost mutex")  //
        ("goal_variables_first", po::bool_switch(&m_values.goal_variables_first)->default_value(false),
         "If this flag is set, variables in goal clauses will be moved to the front of the variable order")  //
        ("initial_state_variables_first",
         po::bool_switch(&m_values.initial_state_variables_first)->default_value(false),
         "If this flag is set, variables in initial state clauses will be moved to the front of the variable order")  //
        // layer building
        ("bidirectional",
         po::bool_switch(&m_values.bidirectional)->default_value(false),
         "If this flag is set, the the bdd layer construction will work in a bidriectional manner")  //
        ("exponential",
         po::bool_switch(&m_values.reverse_layer_building)->default_value(false),
         "If this flag is set, the layers will be constructed exponentially.")  //
        ("share_foundations",
         po::bool_switch(&m_values.share_foundations)->default_value(false),
         "If this flag is set during bidirectional lyer construction, both start nodes of the bidirectional search will be the same")  //
        ("reverse_layer_building",
         po::bool_switch(&m_values.reverse_layer_building)->default_value(false),
         "If this flag is set, the layer bdds will be constructed, beginning from the goal, from last timestep to first")  //
        ("use_layer_permutation",
         po::bool_switch(&m_values.use_layer_permutation)->default_value(false),
         "If this flag is set, the new layer bdd will be constructed using a variable permutation");  //

    po::store(po::parse_command_line(argc, argv, m_option_desc), m_argument_map);
    po::notify(m_argument_map);
}

bool option_parser::check_validity() {
    if ((m_values.encode_cnf + m_values.build_bdd + m_values.build_bdd_by_layer + m_values.build_sdd + m_values.single_minisat +
         m_values.count_minisat + m_values.hack_debug + m_values.cnf_to_bdd) != 1) {
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