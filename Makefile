# Compiler and flags
CC = gcc
CFLAGS = -g # debugging information
CFLAGS += -Wall -Wextra # more warnings
CFLAGS += -Wno-sign-compare # ignore comparison of signed and unsigned integer warning
CFLAGS += -DBOOST_LOG_DYN_LINK # without this the boost log library does not work

# Directories that contain important stuff
SRC_DIR = src
INCLUDE_DIR = include
DEP_DIR = dependencies

# Directories the contain .h header files
INCLUDE_CUDD := -I$(DEP_DIR)/cudd-3.0.0 -I$(DEP_DIR)/cudd-3.0.0/util -I$(DEP_DIR)/cudd-3.0.0/cudd -I$(DEP_DIR)/cudd-3.0.0/st -I$(DEP_DIR)/cudd-3.0.0/mtr -I$(DEP_DIR)/cudd-3.0.0/epd
INCLUDE_SDD := -I$(DEP_DIR)/libsdd-2.0 -I$(DEP_DIR)/libsdd-2.0/include
INCLUDE_ALL := -I$(INCLUDE_DIR) $(INCLUDE_CUDD) $(INCLUDE_SDD)

# Libraries that are used
LIB_CUDD := $(DEP_DIR)/cudd-3.0.0/cudd/.libs/libcudd.a
LIB_SDD := $(DEP_DIR)/libsdd-2.0/build/libsdd.a
LIB_COMMON = -lstdc++ -lm -lpthread -lboost_program_options -lboost_thread -lboost_log -lboost_log_setup

# All of my own cpp files
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/fnf/*.c)

TARGET = main

all:
	make clean
	make build
	make gripper

build: $(CPP_FILES)
	$(CC) $(CFLAGS) $(INCLUDE_ALL) $(CPP_FILES) $(LIB_CUDD) $(LIB_SDD) $(LIB_COMMON) -o $(TARGET)

variable_order:
	./$(TARGET) --cnf_file problem.cnf --cnf_to_bdd
	dot -Tpng befor_reorder.dot -o befor_reorder.png
	dot -Tpng after_reorder.dot -o after_reorder.png

easygripper:
	../downward/fast-downward.py --sas-file easygripper.sas --translate ../downward-benchmarks/gripper-easy/prob01.pddl
	./$(TARGET) --sas_file easygripper.sas --build_bdd --timesteps 5

gripper:
	../downward/fast-downward.py --sas-file gripper.sas --translate ../downward-benchmarks/gripper/prob01.pddl 
	./$(TARGET) --sas_file gripper.sas --build_bdd --timesteps 11

blocks:
	../downward/fast-downward.py --sas-file blocks.sas --translate ../downward-benchmarks/blocks/probBLOCKS-4-0.pddl
	./$(TARGET) --sas_file blocks.sas --build_bdd --timesteps 6

childsnack:
	../downward/fast-downward.py --sas-file childsnack.sas --translate ../downward-benchmarks/childsnack-sat14-strips/child-snack_pfile05.pddl
	./$(TARGET) --sas_file childsnack.sas --build_bdd --timesteps 4

rovers:
	../downward/fast-downward.py --sas-file rovers.sas --translate ../downward-benchmarks/rovers/p01.pddl
	./$(TARGET) --sas_file rovers.sas --build_bdd --timesteps 10

trucks:
	../downward/fast-downward.py --sas-file trucks.sas --translate ../downward-benchmarks/trucks-strips/p01.pddl
	./$(TARGET) --sas_file trucks.sas --build_bdd --timesteps 13

minisat:
	../MiniSat_v1.14_linux problem.cnf minisat.ass

# removes sas problems, cnf formulas and assignments to these files
# also removes graph .dot files and the .png images of those
clean:
	rm -f *.sas *.cnf *.ass *.dot *.png *.o main




