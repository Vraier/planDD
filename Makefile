TARGET = build/planDD

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
	./$(TARGET) --sas_file easygripper.sas --build_bdd --linear --timesteps 5

gripper:
	../downward/fast-downward.py --sas-file gripper.sas --translate ../downward-benchmarks/gripper/prob01.pddl 
	./$(TARGET) --sas_file gripper.sas --build_bdd --linear --timesteps 11

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

movie:
	../downward/fast-downward.py --sas-file movie.sas --translate ../downward-benchmarks/movie/prob27.pddl
	./$(TARGET) --sas_file movie.sas --build_bdd --exact_one_constraint --linear --timesteps 7

minisat:
	../MiniSat_v1.14_linux problem.cnf minisat.ass

# removes sas problems, cnf formulas and assignments to these files
# also removes graph .dot files and the .png images of those
clean:
	rm -f *.sas *.cnf *.ass *.dot *.png *.o main




