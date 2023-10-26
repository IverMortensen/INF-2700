# Assignment 3 
## How to run the program
Generate the names lists used by the natural join test:

	python3 nameGenerator.py

The program asks how many names to generate. Type the number of names you want into the console and press enter. The names will be generated and are automatically ready to be used.

The program can now be compiled with the new names and be ran. This can be done while in assignments-2-3/db2700/ and run the following line:

	make cleanall; make; ./run_front

When the program is running, the implementation of natural join can be tested by typing the following into the program:

	test

---
A simpler I/O test, without any setup, can be ran while in assignments-2-3/db2700/ like this:

	make cleanall; make; ./run_test


# Assignment 2

## How to run the program
Turn on binary search by setting **testBinarysearch** to 1 in schema.c line 44.

The program can be compiled and ran with the following command:

	make cleanall; make; ./run_front


While in the program, create and fill a table with numbers like this:

	make {table_name}

The current table that is made has 2 fields, "Numbers1" and "Numbers2".
To test the binary search, use an equal operand when selecting from the table:

	select * from {tableName} where Numbers1 = 200
	select * from {tableName} where Numbers2 = 600
	select Numbers2 from {tableName} where Numbers2 = 600

The other operands <, >, <=, >= and != uses linear search.

	select * from table where Numbers2 < 100

## Configurations
To change the table that is made with **make table_name**,
go to make_table() function in interpreter.c:734.

If a non sorted table or a table with duplicates
is used, remember to turn off binary search as it 
does not support it.
