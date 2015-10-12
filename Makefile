GPP   = g++ -g -O0 -Wall -Wextra -std=gnu++11
GRIND = valgrind --leak-check=full --show-reachable=yes

all : oc

oc : stringset.o auxlib.o cppstrtok.o
	${GPP} stringset.o auxlib.o cppstrtok.o -o oc

%.o : %.cpp
	${GPP} -c $<

ci :
	git add -u
	git commit

spotless : clean
	- rm oc

clean :
	-rm stringset.o auxlib.o cppstrtok.o *.str

test : oc
	${GRIND} oc * * >test1.out 2>test1.err
	${GRIND} oc foo foo foo foo bar bar bar foo qux baz \
	         >test2.out 2>test2.err

lis : test
	mkpspdf Listing.ps stringset.h stringset.cpp main.cpp \
	        Makefile test1.out test1.err test2.out test2.err
deps:cppstrtok.o stringset.o auxlib.o
# Depencencies.
cppstrtok.o: cppstrtok.cpp stringset.h auxlib.h
stringset.o: stringset.cpp stringset.h
auxlib.o: auxlib.cpp auxlib.h
