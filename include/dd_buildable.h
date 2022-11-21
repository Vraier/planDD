#pragma once

#include <vector>
#include <string>

class dd_buildable {
   public:
    virtual ~dd_buildable(){};

    // clears all old dds and initialalizes num new dds
    virtual void set_num_dds(int num_dds) = 0;

    // check if the bdd is the zero node
    virtual bool is_constant_false(int dd_index) = 0;
    // counts assuming all existing variables are used
    virtual double count_num_solutions(int dd_index) = 0;
    // counts assuming only nvars variables are used
    virtual double count_num_solutions(int dd_index, int nvars) = 0;

    // frees all the nodes under this dd
    virtual void clear_dd(int dd_index = 0) = 0;
    // adds a clause to the root node
    virtual void add_clause_primitive(std::vector<int> &clause, int dd_index = 0) = 0;
    // adds an exact one constraint to the root node
    virtual void add_exactly_one_primitive(std::vector<int> &variables, int dd_index = 0) = 0;
    // adds a dnf to the bdd. it has the form 'g or (a^b^c) or (x^y)'
    virtual void add_dnf_primitive(std::vector<std::vector<int>> &dnf, int dd_index = 0) = 0;

    // creates variables until i variables exist.
    // this is important for variable ordering
    // one has to create variables before he can order them
    virtual void create_ith_var(int i) = 0;

    virtual void set_variable_group(int low, int size) = 0;


    // Functions for building the dd timestep by timestep
    // permutes the variables in source bdd according to the given permutation
    // write the result into destionation bdd. the old bdd in destination gets freed and is lost
    virtual void permute_variables(std::vector<int> &permutation, int source_bdd, int destination_bdd) = 0;
    // conjoins bdd a and b and stores the result. the old destination bdd get freed and is lost
    virtual void conjoin_two_dds(int dd_a, int dd_b, int dd_result) = 0;

    // regarding variable order
    // stops automatic reordering and reactivates it
    virtual void disable_reordering() = 0;
    virtual void enable_reordering() = 0;
    // reorders the variables with the reorder_shift_converge method
    virtual void reduce_heap() = 0;

    virtual std::string get_short_statistics(int dd_index = 0) = 0;
    virtual void print_info() = 0;
};