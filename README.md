# Tiger compiler
A Tiger language compiler made in C for MIPS-32 architecture, developed for the compiler discipline.

## How to compile

Just type `make`.

## How to use
Usage: `./tc [flags]`.

Flags:
```bash
-h    		prints usage guide
-p [path]	sets the input file path
-o    		sets the name of the output binary
-a    		prints the abstract syntax tree
-i    		prints the intermediate representation
-c    		prints the canonical intermediate representation tree
-s    		prints the generated assembly code before regs allocation
```

## New construction added to language

In this implementation, `do while` construction was added.

### Usage

Note that for the structure to work correctly, the condition must be in parentheses. This restriction was necessary to minimize the amount of shift/reduce in the grammar. 

```
do {
	"body"
} while (condition);
```

## How to test

There are some considerations to be made about the tests. 

1. All tests in set 1 must fail
2. All tests in set 2, with the exception of test 9, must pass. However, test 2 of this set will not be performed due to the following restriction inserted: nil records cannot be compared with filled records.
3.  All tests in set 3, with the exception of test 10, must pass. The reason why test 10 should fail is the presence of the ">>" operator, however, this operator is not defined by default in the Tiger language and must be implemented as a new structure. 

Usage: 
```bash
cd testcases/ && ./suite_test.sh
```