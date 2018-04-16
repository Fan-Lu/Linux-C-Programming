Notes: 
1.There are four deadlock scenarios shown in four folders and each scenario is described in the README file in each folder;  
2.Testing Procedure (for example, if want to test scenario 1):  
  ~$ cd TESTCASE1  
  ~$ make all  
  ~$ sudo insmod driver_testcase1.ko  
  ~$ sudo ./TESTCASE1  
  and you will see a deadlock scenario. For most of the case, a deadlock will occur. If the deadlock doesn't occur, you can implement  
  ~$ sudo ./TESTCASE1  
  for several times until deadlock occurs.  
  To remove the devcice, just excutes  
  ~$ sudo insmod driver_testcase1.ko  
  To clean up the compiled files, excute: 
  ~$ make clean   
3.for some reasons, on my laptop, I can not compile the code if using <asm/uaccess.h> head file, so I change it to <linux/uaccess.h>,  
  if you cannot compile the file, you can change this head file to <asm/uaccess.h>;

