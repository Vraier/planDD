# planDD

This Project aims to construct BDDs and SDDs from planning problems.
The DDs represent the set of optimal plans, that solve the planning problem.
Multiple strategies are implemented to allow for efficient DD construction.
Representing a set of plans as a DD allows for advanced queries on this set.
Some of the possible queries include:
  - Counting the number of solutions
  - Selecting a plan uniform at random
  - Checking a certain actions must hold in every plan

This project was created in the context of my master thesis.

# Build

To build the project, one has to compile the BDD and SDD library first.

Inside the folder planDD/contrib/cudd-3.0.0 run
```
./configure
make
```
Inside the folder planDD/contrib/libsdd-2.0 run
```
scons
```
For more information, check their README files.

To compile planDD create and move to the planDD/build directory and run
```
cmake ..
make
```

# Dependencies

I use the CUDD library (https://github.com/ivmai/cudd) and the SDD library (http://reasoning.cs.ucla.edu/sdd/) to build the BDDs and SDDs.
Their license files can be found in the corresponding folder in planDD/contrib
