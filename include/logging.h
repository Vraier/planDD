#ifndef LOGGING_H
#define LOGGING_H

#include <boost/log/attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <string>

namespace log_level = boost::log::trivial;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(global_logger, boost::log::sources::severity_logger<boost::log::trivial::severity_level>)

#define LOG_MESSAGE(severity) \
    LOG_LOCATION;             \
    BOOST_LOG_SEV(global_logger::get(), severity)

// Macro that sets the global file and line variable of the logger
#define LOG_LOCATION                                                                   \
    boost::log::attribute_cast<boost::log::attributes::mutable_constant<int>>(         \
        boost::log::core::get()->get_global_attributes()["Line"])                      \
        .set(__LINE__);                                                                \
    boost::log::attribute_cast<boost::log::attributes::mutable_constant<std::string>>( \
        boost::log::core::get()->get_global_attributes()["File"])                      \
        .set(path_to_filename(__FILE__));                                              \
    boost::log::attribute_cast<boost::log::attributes::mutable_constant<std::string>>( \
        boost::log::core::get()->get_global_attributes()["Function"])                  \
        .set(__FUNCTION__);

void initialize_logging();

// Convert file path to only the filename
std::string path_to_filename(std::string path);

#endif