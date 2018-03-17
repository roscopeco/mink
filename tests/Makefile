# Makefile for unit tests. These obviously aren't cross-compiled,
# they're run on the build platform...
#
CC	= gcc
CFLAGS	= -Wall -O0 -std=c11 -Iunity -I../include $(EXTRA_CFLAGS)

SHELL	= bash
RUBY 	= ruby
RM 	= rm -rf
MKDIR	= mkdir

SOURCES = $(wildcard test_*.c)
TESTS 	= $(patsubst test_%.c,test_%.test,$(SOURCES))
PATHR	= results/
RESULTS = $(patsubst test_%.c,$(PATHR)test_%.txt,$(SOURCES))

all: test

test: $(TESTS) $(RESULTS)
	@echo "-----------------------"
	@echo "IGNORES:"
	@echo "-----------------------"
	@echo `grep --no-filename -s IGNORE: $(PATHR)*.txt`
	@echo "-----------------------"
	@echo "FAILURES:"
	@echo "-----------------------"
	@echo `grep --no-filename -s FAIL: $(PATHR)*.txt`
	@echo
	@echo "DONE"

$(PATHR)%.txt: %.test $(PATHR)
	-./$< > $@ 2>&1

%_Runner.c : %.c
	$(RUBY) unity/generate_test_runner.rb $< $@

%.test:%.c %_Runner.c unity/unity.c
	$(CC) $(CFLAGS) -o $@ $^

$(PATHR):
	$(MKDIR) $(PATHR)

.PHONY: clean
clean:
	$(RM) *.test $(PATHR)
 
