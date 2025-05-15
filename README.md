# Proof Of Concept

## Dependency
This Proof-Of-Concept project uses “cpprest” library to build the HTTP Servers. The system where this project is being built needs have this library installed. Installation procedure will depend upon the operating system involved. For example, in a Debian/Ubuntu system, it can be installed using following command, 

  	$ sudo apt install libcpprest-dev

For detailed installation procedure in other systems, the official documentation would be helpful.
 

## Build the Web-Server
Create build folder	$ cmake -B <build-folder>
Compile the project	$ cmake --build <build-folder>

## Start the Web-Server
From a terminal 	$ ./build/httpServerProto

## Quit the Web-Server
Just press “Enter” in the terminal where the server is running.  	
