# Define (atleast) the following targets: all, clean
# all must build the executable file named mapreduce.
# clean must remove all object files and executable files created.
# Look at the introduction doc for information on how make a Makefile.
# This make file must build according to the structure described in the
# introduction doc, if it doesn't you will get a ZERO!
CFLAGS = -Wall -Werror -pedantic -Wextra -Iinclude

all: clean utf
	
utf:
	@mkdir -p build
	@mkdir -p bin
	gcc $(CFLAGS) -c src/utfconverter.c -o build/utfconverter.o -lm
	gcc $(CFLAGS) -o bin/utf build/utfconverter.o -lm

debug:
	@mkdir -p build
	@mkdir -p bin
	gcc $(CFLAGS) -g -c src/utfconverter.c -o build/utfconverter.o -lm
	gcc $(CFLAGS) -g -o bin/utf build/utfconverter.o -lm


clean:
	rm -rf bin
	rm -rf build
