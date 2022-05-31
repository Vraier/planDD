cmake_minimum_required(VERSION 3.16)
project(planDD)

add_library(cudd STATIC IMPORTED GLOBAL)

set_property(TARGET cudd PROPERTY
	IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cudd-3.0.0/cudd/.libs/libcudd.a")

target_include_directories(cudd SYSTEM
	INTERFACE
		"contrib/cudd-3.0.0/cudd"
		"contrib/cudd-3.0.0/util"
		"contrib/cudd-3.0.0")


add_library(libsdd STATIC IMPORTED GLOBAL)

set_property(TARGET libsdd PROPERTY
	IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/contrib/libsdd-2.0/build/libsdd.a")

target_include_directories(libsdd SYSTEM
	INTERFACE
		"contrib/libsdd-2.0"
		"contrib/libsdd-2.0/include")

set(BOOST_VERSION 1.71.0)

find_package(Boost ${BOOST_VERSION} EXACT REQUIRED
	COMPONENTS
		program_options
		log)

#target_link_libraries(cudd
#	INTERFACE "/home/markus/code/cudd/cudd/.libs/libcudd.a")

add_executable(planDD)
target_sources(planDD
	PRIVATE
		"src/bdd_container.cpp" "src/planning_logic_formula.cpp" "src/cnf_encoder.cpp" "src/dd_builder_conjoin_order.cpp"
		"src/dd_builder.cpp" "src/dd_builder_variable_order.cpp" "src/logging.cpp" "src/main.cpp"
		"src/options.cpp" "src/sas_parser.cpp" "src/sdd_container.cpp"
		"src/fnf/compiler.c" "src/fnf/fnf.c" "src/fnf/io.c" "src/fnf/utils.c")

target_include_directories(planDD PRIVATE "include")

target_link_libraries(planDD
	PRIVATE cudd libsdd Boost::program_options Boost::log)

target_compile_options(planDD
	PRIVATE "-Wall" "-Wextra" "-Wno-sign-compare")
