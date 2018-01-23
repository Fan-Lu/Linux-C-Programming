# Linux-C-Programming

Execution Sequence:
	1. First execute MapperMakefile;
	2. Second execute ReducerMakefile;
	3. Third execute CombinerMakefile;

Attention:
	1. If execute CombinerMakefile first, please execute it twice to get desired results;
	2. To execute ReducerMakefile, must make sure mapper_output.txt exist.
	3. There are three makefiles in the folder, i.e., MapperMakefile, ReducerMakefile, CombinerMakefile. I do this because there are three programms in the assignment for us to code and I want to make sure each programm runs without no bug.