# variable
OBJECTS = lex.yy.o errormsg.o ../util/util.o

# executable
all: lextest
.PHONY: all

lextest: lextest.o $(OBJECTS)
	cc -g -o $@ lextest.o $(OBJECTS)

# objects
lextest.o: lextest.c tokens.h errormsg.h ../util/util.h
lex.yy.o: lex.yy.c tokens.h errormsg.h ../util/util.h
errormsg.o: errormsg.c errormsg.h ../util/util.h
util.o:../util/util.c ../util/util.h

# lex
lex.yy.c: tiger.lex
	lex tiger.lex

.PHONY: clean
clean:
	rm -f *.o lex.yy.c lextest
	rm -f ../util/*.o 

