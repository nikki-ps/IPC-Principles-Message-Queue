CPSC351-02-Project 1: IPC Principles

TEAM MEMBERS: 
Zach Serna, serna648@csu.fullerton.edu, Section 02
Nicole Serna, nicoleserna@csu.fullerton.edu, Section 02

PROGRAMMING LANGUAGES USED: C++

EXECUTION OF PROGRAM:
1. Place all the files into a database, either via tar or copying the files into a 
new database.
Note: We had problems getting the tar to work properly so if the tar does not
unzip properly just create a new directory and copy makefile,msg.h,sender.cpp
and recv.cpp into it.
2. Invoke the makefile by using the "make" command to compile recv.cpp and sender.cpp
3. Use the command ./recv to call recv.cpp; it will be on standby
4. In another console window, use the command ./sender (FILENAME) to call 
sender.cpp; it will then execute. The file will be sent to shared memory which
recv.cpp will be able to access.


EXTRA CREDIT: No


CONTRIBUTIONS:

We divided the work based on the two files: Sender.cpp and recv.cpp. Zach worked on the 
recv.cpp file while Nicole worked on the sender.cpp file. We then mutually worked
together to fix any bugs we encountered when trying to compile the programs. The 
signal anbd makefile were mutually made as well; even though those were overall
trivial amounts of work.

NOTES: "Keyfile.txt" is created in the code so manual creation is unnecessary.
Attempting to run the program multiple times in a row without deleting the 
previously created files will lead to unexpected behaviors. 




