# CommandParser
This project is about implementing the Command line interface in C.
Developer can integrate the library with the application and registered his own customized commands.

This Project gives developer Linux like command like interface to interact with the application.

See The testapp.c to learn the usage of the library.
#include libcli.h and libtlv.h in your application to use the library.
Compile your application by linking it with licli.a library using -lcli. See Makefile for help.

Steps :
1. Run 'make' to compile the library.
2. Run 'sudo make install' to install the library in /usr/local/lib path. 
3. Test "echo $LIBRARY_PATH" whether /usr/local/lib is included in the path. If yes, then step 4 and 5 are not required.
4. In ~/.profile file, add line :  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
5. After step 4, run 'sudo ldconfig'. 
6. Run aplication executable and enjoy the CLI.
