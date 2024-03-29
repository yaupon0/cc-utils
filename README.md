# cc-utils
Collection of C example, utility programs and C headers. Non-commercial license unless there is a specific BSD style license in a file.

- **thread.c** an example of lock free synchronization without any synchronization operations - works on x86. 

- **owq.h and owq_test.c**  A **lock free queue** (one producer, one consumer) called a one-way-queue (owq). The queue is restricted to one data type which is selected at compile time. To use owqs with multiple different element types there are three options: (1) include the header in multiple files, each with a data type, and export some wrapper, (2) use void * or unions as the data type, (3) use the sed trick in the Makefile or something similar.  The code depends on strong memory ordering and has been tested on Intel processors only. 

The utility is presented as an include file with some static, inline, functions. A discussion of how to use it is in the comments of owq.h and the owq_test.c file is both an example and test code. Run "make owq_test" to build. There is a sed command in the makefile to provide C style generic code. 

- **Paxos.lua** A simulator for Paxos that shows the livelock problem. 

- **markov.c**  A C version of the Lua Markov text generator. Completely useless, although it does show off C generics

- **include/dlinklist.h** A generic C double linked list (see use of sed in Makefile). 

- **include/mmalloc.h** Malloc with exit on fail so callers don't have to check the result - for when malloc failures are non recoverable. 

- **include/hash.h** some standard hash functions plus a variant needed for the markov program



