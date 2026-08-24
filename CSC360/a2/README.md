My mts.c program should be fully working. 
If given a properly formatted input file it will seperate each line into a train thread per line.
These trains will have a direction, a priority, a loading time and a crossing time which are all initialized using a struct. 
On top of each train thread there is a controller thread that manages the scheduling for all the trains.

The scheduling logic is as follows:
If there is one train with priority in either direction then that train is scheduled first.
If there is a train with high priority in both directions that it will alternate direction, if no last direction is known then it will schedule the east bound train first. 
If there are no high priority trains ready then the same process is used to determine the next low priority train. 
Lastly it updates the next direction, briefly waits and decreases the trains remaining by one.

The train thread simulates loading time, locks the station mutex and prints it's status.
After this the train enqueues itself and signals to the controller that it is ready.
It waits for its turn to unlock the station mutex.
once it starts moving it locks this mutex again and simulates crossing.
At the beggining and end of the crossing it prints its status.

I was having some trouble with high priority trains leaving the station after low priority trains with the same load time.
This was because the controller thread would check the train readiness the instead a train is ready.
This caused low priority trains that were ready very slightly earlier to be queued first.
To solve this issue I added a 1000 nanosecond increase to low priority train load times.
This seemed to fix the issue, now all high priority trains that load at the same time as low priority trains will depart first. 

The make file I have will compile the mts.c on 'make'. 'make clean' will remove the output file and other files created by the program. 
To run the program first compile with make and then use ./mts input.txt