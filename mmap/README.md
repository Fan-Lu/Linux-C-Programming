# Producer Comsumer Problem
Rewrite the combiner program using mmap system call with anonymous mapping (no backingfile).

# Excution Procedure
1.  To excute the code, please makesure that mmap makefile exist. If not, please use the command:  
    gcc -o mmap -pthread mmap.c   
2.  Second, you need to delete the reducer_output.txt to have the correct results;  
3.  If all are done, then excute the following command:  
    ./mmap 4 7 <input.txt  
    Arguments in this command:  
    4 means the buffer size in the code;  
    7 means the number of users in the input.txt. you can change this number but make sure that the value of this argument corresponds to       the number of users in the  input.txt;  
    input.txt is the input file in the directoryl  
