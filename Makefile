GPP   = g++ -g -O0 -Wall -Wextra -std=gnu++11
GRIND = valgrind --leak-check=full --show-reachable=yes
DEPSFILE = Makefile.deps
MKDEPS = g++ -MM -std=gnu++11
CSRC = stringset.cpp auxlib.cpp cppstrtok.cpp
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
deps : ${CSRC}
	@ echo "# ${DEPSFILE} created 'date' by ${MAKE}" >${DEPSFILE}
	${MKDEPS} ${CSRC} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${MAKE} --no-print-directory deps
# Depencencies.
cppstrtok.o: cppstrtok.cpp stringset.h auxlib.h
stringset.o: stringset.cpp stringset.h
auxlib.o: auxlib.cpp auxlib.h
