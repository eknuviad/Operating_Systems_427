 To run the simple user level thread program, do the following:
    - In a terminal, run 'make sma'.
    - After compilation, an unilikable sma.o file will be created
    - Use this sma.o file by typing in the command line
        : gcc sut.o <test-you-want-to-run>.c 
    - To start the test type the ff and hit enter
        : ./a.out
    - Run make clean to remove executables

Assignment Summary
3/6 tests are able to run ie Test 1, 2, 6. All other tests seemed to fail
becuase I couldn't resolve issues with linked list pointer allocation 
and off by one deallocation. But all methods except sma_realloc are implemeted.

What was Accomplished
1. My code compiles successfully without bugs.
2. Code successfully passes Test 1, Test 2, Test 6.
3. Methods implemeted (malloc, free, mallinfo,)
3. Issues with passing Test 4, 5 can be resolved by fixing the add_free_block methods
    as this method has problems where the nodes are either not initialised properly
    or cant allocate the next node to the list. I managed to resolve the initialisation
    roblem but I also had the issue of an off by one pointer deallocation whereby
    Test 3 was allocating blocks of size 1 or 0, indicating that the tags of the blocks
    were being deallocated or stored in a wrong position for each block I allocated.
    This stumped my progress on the further parts. Because I couldnt fix the next node 
    allocation in time, I "commented out" the right method to initialise the node. This
    can be seen in sma.c
