#include "logging.h"

#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

void initialize_logging() {
    // print everything to console
    boost::shared_ptr<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>> console_sink =
        boost::log::add_console_log(
            std::cout,
            boost::log::keywords::format =
                (boost::log::expressions::stream
                 << "[" << boost::log::expressions::attr<boost::log::attributes::timer::value_type>("Uptime") << "]"
                 << "[" << boost::log::trivial::severity
                 << "]"
                 //<< "[" << boost::log::expressions::attr<std::string>("File") << ":"
                 //<< boost::log::expressions::attr<int>("Line") << ":"
                 //<< boost::log::expressions::attr<std::string>("Function") << "()]"
                 << " " << boost::log::expressions::smessage));

    // TODO, this is supposed to decrease performance significantly (i measured ~1-2sec) but i need it to dont loose
    // output on log files
    console_sink->locked_backend()->auto_flush(true);

    boost::shared_ptr<boost::log::core> core = boost::log::core::get();
    // timer that counts from the beginning of the program
    core->add_global_attribute("Uptime", boost::log::attributes::timer());
    // New attributes that hold filename and line number
    core->add_global_attribute("File", boost::log::attributes::mutable_constant<std::string>(""));
    core->add_global_attribute("Line", boost::log::attributes::mutable_constant<int>(0));
    core->add_global_attribute("Function", boost::log::attributes::mutable_constant<std::string>(""));

    // core->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

// Convert file path to only the filename
std::string path_to_filename(std::string path) { return path.substr(path.find_last_of("/\\") + 1); }
