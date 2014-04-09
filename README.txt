README file.

Name : Srinivasan Rengarajan
UBID : 50097996

                                                  MULTITHREADED WEB SERVER

This file contains instructions and design criteria adopted while coding the
Opearting systems project -1 : Multithreaded web server.

This project with coded in C++ and the header file <pthread.h> namely POSIX thread
library is used for implementing the threading functionality in the project.

Brief sequence of the project:
The program contains 2+n threads, n= no of worker threads.
The threads other than worker threads are scheduling and queueing threads.

Scheduling thread:
This thread is responsible for taking the elements out of the queue and scheduling
them as FCFS or SJF scheduling depending on the choice of the user


Queuing thread:
This thread is repsonsible for queuing the requests in order and then listening for 
incoming client requests. The time between the queuing and scheduling thread is 60 sec.
This is referred to as the Queuing time.

Worker Threads:
These threads are set to the value of 4 by default. These threads execute in order and are protected
by mutexes so that synchronization is achieved. 

Brief Working of algorithm:
1. The thread described above operate in synchronization in order to allow multithreaded access
to the web server. 
2. The request is brought into the waiting queue and every request waits for 60 sec before
being put into the ready queue.
3. After the requests are ready in the ready queue, they are scheduled by the scheduling thread
with either the FCFS or the SJF algorithm as specified by the user.
4. The worker threads are assigned a request one by one from the queue and they are processed 
in order. In case of SJF algorithm, the queue with the least file size is scheduled first.
All these requests are assumed to be atmoic.
5. If the server is operated in Debugging mode, then only one thread can operate at a time.
6. If the user enables the logging mode, then all the current requests with their
current time stamps, lengths and last modified date,time stamp are written to a LOG FILE which
is maintained by the server.


Advantages of the algorithm:
Context switches are avoided by using MUTEXES and conditional variables.For instance, when a thread
finished executing a job, it signals the scheduling thread that one worker thread is free. Similarly,
there is a sunchronization eshtablished between the scheduling thread and the queuing thread since 
they are accessing a shared variable, the queue. Conditional variables are more effective in 
synchronizing the requests as they avoid race condition.

Guide to running the program

g++ final.cc -pthread -o final

Resources used:
1. www.stackoverflow.com

2. Beej's guide to network programming

3. http://kturley.com/simple-multi-threaded-web-server-written-in-c-using-pthreads/












 		




