 To run the simple user level thread program, do the following:
    - In a terminal, run 'make sut'.
    - After compilation, an unilikable sut.o file will be created
    - Use this sut.o file by typing in the command line
        : gcc sut.o <test-you-want-to-run>.c -l pthread
    - To start the test type the ff and hit enter
        : ./a.out

    - Run make clean to remove executables

