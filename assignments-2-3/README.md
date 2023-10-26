# How to run the program
Compile and run program while in assignments-2-3/db2700/:

	make cleanall; make; ./run_front

## Commands in the program
Show availeble commands:

	help

To test Natural-join and Block Natural-join:

	test

To change the table that is made with make "tableName",
go to make_table() function in interpreter.c:731.

The current table made has 2 fields, "Numbers1" and "Numbers2".
To use binary search on the table the equal operand can be used
on one of the fields like this:

	$ select * from {tableName} where Numbers1 = 200
	$ select * from {tableName} where Numbers2 = 600
	$ select Numbers2 from {tableName} where Numbers2 = 600

The other operands <, >, <=, >= and != uses linear search.

	$ select * from table where Numbers2 < 100

If a non sorted table or a table with duplicates
is used, remember to turn of binary search as it 
does not support it.
It can be done by uncommenting "doBinarySearch = 0"
on line 837 in schema.c
