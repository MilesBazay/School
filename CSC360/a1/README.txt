CSC 360 - Programming Assignment 1 

I have implemented the follwing features into my program:

ls (Working wihtout bonus part): Will show the contents of a specified file or the file that you are currently working in if no file is specified.

cd (fully working): Takes commands like cd <directory> or cd ../.. etc

Signal handling: 

CTRL + C interrupts the foreground process if it is running or moves to a new line without processing the already typed commands. Does not terminate the shell itself.

CTRL + D terminates the shell

exit is the same as CTRL + D but prints a goodbye message

Background command execution: Takes in a bg <command> and starts a running proccess without interrupting the users foreground process. Also prints a terminated message when completed.

Background job listing: lists the number of background jobs and their id's without listing any previously terminated processes

pwd: prints the working directory

date: prints the date and time

make compiles all project files

uname -a, ps -a and ls -alh gives same input as in a normal shell

executes arbitrary programs and prints <name>: No such file or directory