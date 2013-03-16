SRCDIR = ./src ./src/portable
IDIR = ./include
LIBS=

SRC = $(foreach d,$(dir $(SRCDIR)),$(wildcard $(d)*.c))
IDIR := $(patsubst %,-I%,$(subst :, ,$(IDIR)))

ODIR = obj

CC = gcc
CCFLAGS += -Wall -pedantic

OBJ = $(patsubst %,./$(ODIR)/%,$(notdir $(SRC:.c=.o)))

space = $(empty) $(empty)

all: $(OBJ) vm

$(OBJ): $(SRC)
	$(eval CUR_SRC := $(notdir $(@:.o=.c)))
	$(eval CUR_SRC := $(filter $(foreach file,$(CUR_SRC), %/$(file)), $(SRC)))
	$(CC) $(IDIR) $(CCFLAGS) -c $(CUR_SRC) -o $@

vm: $(OBJ)
	@echo "Link PTTK91 VM"
	@echo "=================================================================="
	$(CC) $(CCFLAGS) $(IDIR) $^ $(LIBS) -o ./$@

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o 

