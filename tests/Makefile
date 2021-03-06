# Makefile for unit tests. These obviously aren't cross-compiled,
# they're run on the build platform...
#
CC	= gcc -c
LD	= gcc
CFLAGS	= -Wall -O0 -std=c11 -I. -I.. -I../include $(EXTRA_CFLAGS)
DEPEND	= gcc -MM -MG -MF
SHELL	= bash
RUBY 	= ruby
RM 	= rm -rf
MKDIR	= mkdir

SOURCES = $(filter-out %_Runner.c, $(wildcard test_*.c))
TESTS 	= $(patsubst test_%.c,test_%.test,$(SOURCES))
PATHR	= results/
RESULTS = $(patsubst test_%.c,$(PATHR)test_%.txt,$(SOURCES))

# TODO fix this later - should be using dependencies, not cleaning every time!
all: clean test

.PHONY: test
test: $(TESTS) $(RESULTS)
	@echo "-----------------------"
	@echo "IGNORES:"
	@echo "-----------------------"
	@grep --no-filename -s IGNORE: $(PATHR)*.txt; true
	@echo "-----------------------"
	@echo "FAILURES:"
	@echo "-----------------------"
	@grep --no-filename -s FAIL: $(PATHR)*.txt; true
	@echo
	@grep --no-filename -s '[0-9]* Tests' $(PATHR)*.txt; true
	@echo
	@echo "DONE"

$(PATHR)%.txt: %.test $(PATHR)
	-./$< > $@ 2>&1

%.d: %.c
	$(DEPEND) $@ $<

%_Runner.c: %.c
	$(RUBY) generate_test_runner.rb $< $@

%.o: %.c
%.o: %.c %.d
	$(CC) $(CFLAGS) -o $@ $<

%.test:%.o %_Runner.o unity.o
	$(LD) -o $@ $^

$(PATHR):
	$(MKDIR) $(PATHR)

.PHONY: clean
clean:
	$(RM) *.test $(PATHR) *.d *.o *_Runner.c

.PRECIOUS: %.d
.PRECIOUS: %.o
.INTERMEDIATE: $(PATHR)%.txt
