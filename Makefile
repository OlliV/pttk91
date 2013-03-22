include ./config

SRCDIR = ./src/
ifeq ($(TARGET),LINUX)
	SRCDIR += ./src/portable/linux/
endif

IDIR = ./include ./src
CONFIG_H = ./include/config.h
LIBS=

SRC = $(foreach d,$(dir $(SRCDIR)),$(wildcard $(d)*.c))
IDIR := $(patsubst %,-I%,$(subst :, ,$(IDIR)))

ODIR = obj

CC = gcc
CCFLAGS += -Wall -pedantic

OBJ = $(patsubst %,./$(ODIR)/%,$(notdir $(SRC:.c=.o)))

space = $(empty) $(empty)

all: config $(OBJ) vm

config: $(CONFIG_H)

$(CONFIG_H):
	@cp ./include/config.template $(CONFIG_H)
	@echo "#define VM_PLATFORM $(TARGET)" >> $(CONFIG_H)
	@echo "#define VM_DEBUG $(VM_DEBUG)" >> $(CONFIG_H)
	@echo "#define VM_CODE_AREA_RW $(VM_CODE_AREA_RW)" >> $(CONFIG_H)
	@echo "#define VM_DATA_ALLOW_PC $(VM_DATA_ALLOW_PC)" >> $(CONFIG_H)
	@echo "#endif" >> $(CONFIG_H)

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
	rm -f $(ODIR)/*.o $(CONFIG_H)

