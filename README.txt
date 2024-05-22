README

#Limited XE Assembler

Names:

    Aria Kafie, Htet Hnin Su Wai Class Accounts: cssc4055, cssc4043 Class Information: CS 530, Spring 2024 Assignment Information: Assignment #2, XE assembler program File name: README

File manifest:

    main.cpp optab.cpp optab.h parse.cpp parse.h types.cpp types.h test1.sic test2.sic test3.sic makefile README.txt SDD.txt

Compile Instructions:
    
    to compile simply navigate to the directory containing the makefile and associated c++ source files, and enter the command 'make'

Operating Instructions(include any/all command line options/arguments, required inputs,etc):

    Prepare your SIC/XE assembly source code file (.sic extension), and pass them to the program as command line arguments as follows:

    asxe source_1.sic source_2.sic ... source_n.sic
    
    For every file 'prog.sic', the program will generate the following files:

    prog.l: Listing file containing assembled instructions and addresses
    prog.st: SYMTAB file containing Symbol table and literal table information

List/description of novel/significant design decisions:

    This program makes use of error handling to detect invalid file paths, and displays a friendly message in case of user-error.
    Our program also features a built in arithmetic expression parser which uses classic C/C++ style operator precedence and left to right associativity to evaluate expressions unambiguously

List/description of all known deficiencies or bugs:

    There are no currently known deficiencies or bugs within the program, as we have extensively tested and debugged the program across a wide range of test cases.

Lessons learned:

    Appropriate selection and use of table/list data structures
    Object oriented programming
    Structuring of a c++ program across multiple c++ and header files
    Understanding of SIC/XE architecture
    Two pass parsing algorithm
    Comprehensive Testing and debugging
    Error Handling
    Documentation

