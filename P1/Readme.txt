V00904763
CSC 360 A01/T01
Soyun Lee

How to compile and run your code:
    1. Save linked_list.c, linked_list.h, main.c, test.c and Makefile in the same directory
    2. Run and compile (compiled file included in submission):
        $ gcc test.c -o test
        $ ./test
    3. Update max number of processes (i.e. linux.csc.uvic.ca server):
        $ ulimit -u 80
    4. Run the main program:
        $ make
        $ ./pman
    
Below are six main functions built in the program:
    $ bg test
        runs test.c
    $ bglist 
        displays a list of processes running
    $ bgkill <pid>
        terminates a program with <pid>
    $ bgstop <pid>
        stops a program with <pid>
    $ bgstart <pid>
        continues a program with <pid>
    $ pstat <pid>
        displays a program with <pid>'s stat:
            -comm: (filename)
            -state: process state
            -utime: scheduled time in user mode
            -stime: scheduled time in kernel mode
            -rss(Resident Set Size): number of pages the process has
            -voluntary_ctxt_switches: number of voluntary context switches 
            -nonvoluntary_ctxt_switches: number of involuntary context switches 