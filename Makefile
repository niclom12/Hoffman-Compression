# This was taken from 314 Tut 2 and modified accordingly

COMPILER ?= $(GCC_PATH)gcc

#All implicit rules include ‘$(CFLAGS)’ among the arguments given to the compiler.
CFLAGS ?= -O2 -Wall $(GCC_SUPPFLAGS) #-DDEBUG

#Extra non-library flags to give to compilers when they invoke the linker (‘ld’), e.g., -L. 
LDFLAGS ?= -g

#Library flags or names given to compilers when they invoke the linker, ‘ld’. 
LDLIBS = -lm

#Variable to store the name that the executable should be called once build 
EXECUTABLE = bin/compression

#Get a list of all the source and object files
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=obj/%.o)

#The rule called all is automatically executed when make is executed. 
#In this case it links all the objects in ${OBJS} and all the libraries in ${LDLIBS} into an executable called ${EXECUTABLE}.  
all: $(OBJS) | bin
	$(COMPILER) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS)

#Static pattern rule: target is matched by the target-pattern (via a % wildcard, i.e., obj/%.o). 
#The match is then substituted into the prereq-pattern (src/%.c), to generate the target's prereqs.
obj/%.o: src/%.c | obj
	$(COMPILER) $(CFLAGS) -o $@ -c $<
#Rule to create the obj directory if it does not exist 
obj:
	mkdir -p $@
# Rule to create the bin directory if it does not exist 
bin:
	mkdir -p $@

#Execute by calling: make clean
clean:
	rm -f obj/*
	rm -f *.log
	rm -f $(EXECUTABLE) 