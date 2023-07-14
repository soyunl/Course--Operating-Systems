# Course--Operating-Systems
Completed projects from Operating Systems course. Further information about the course can be found [here](https://heat.csc.uvic.ca/coview/course/2022091/CSC360)

<h2>Projects</h2>

<h3>p1: Process Manager</h3>
How to compile and run:
```bash
$ gcc test.c -o test
$ ./test
$ ulimit -u 80
$ make
$ ./pman
```
Below are the six main functions built in the program:
``` bash
$ bg test # runs test.c in the background
$ bglist # displays a list of all background processes
$ bgkill <pid> # terminates the process with the given pid
$ bgstop <pid> # stops the process with the given pid
$ bgstart <pid> # continues the process with the given pid
$ pstat <pid> # displays a program with the given pid's status where the status is:
              # - comm: (filename)
              # - state: process state
              # - utime: scheduled time in user mode
              # - stime: scheduled time in kernel mode
              # - rss (resident set size): number of pages the process has
              # - voluntary_ctxt_switches: number of voluntary context switches
              # - nonvoluntary_ctxt_switches: number of non-voluntary context switches
```

<h3>p2: Airline Check-in System</h3>
How to compile and run:
```bash
$ make
$ ./ACS inputfile.txt
```
Input file with any invalid value (i.e. arrival time or service time with negative value) will immediately terminate the program with an error message. Example input files are provided. 

<h3>p3: Simple File System</h3>
How to compile and run:
```bash
$ make
$ ./diskinfo <image file.IMA>
$ ./disklist <image file.IMA>
$ ./diskget <image file.IMA> <file from root directory>
$ ./diskput <image file.IMA> <file from linux iwth/without subdirectory>
```
Error messages will be displayed if the input does not match the expected format. Part 4 is incomplete and will only work for basic cases (such as mapping a filename, etc.) as well as error handlings.
