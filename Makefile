# The Makefile based on project from https://github.com/jtilly/mex and rewritten

MATLAB_DIR = /opt/R2018a

OBJ_DIR    = obj
BIN_DIR    = bin

TARGET_NAME = scma_log_mpa_mex
TARGET      = $(TARGET_NAME).$(EXT)

RM          = rm

# compiles mex files using gcc
CC = gcc
LD = gcc

# compiler flags for gcc
CFLAGS = -O3 -std=c99 -fpic -fno-omit-frame-pointer -pthread -Wall

CPPFLAGS = -MMD -MP

# to use the intel compiler instead, uncomment CC and CCFLAGS below:

# compiles mex file using the intel compiler
# CC = icpc

# compiler flags for intel compiler
# CCFLAGS = -O3 -fPIC -D__amd64

# Figure out which platform we're on
UNAME = $(shell uname -s)

# Linux
ifeq ($(findstring Linux,${UNAME}), Linux)
	# define which files to be included
	CINCLUDE = $(MATLAB_DIR)/extern/include
	LDFLAGS= -pthread -Wl,--no-undefined -Wl,-rpath-link,$(MATLAB_DIR)/bin/glnxa64 -shared -L$(MATLAB_DIR)/bin/glnxa64 -lmx -lmex -lmat -lm -lstdc++
	# define extension
	EXT = mexa64
endif

# Mac OS X
ifeq ($(findstring Darwin,${UNAME}), Darwin)
	# define which files to be included
	CINCLUDE = -L$(MATLAB_DIR)/bin/maci64
	LDFLAGS= -pthread -Wl,--no-undefined -Wl,-rpath-link,$(MATLAB_DIR)/bin/maci64 -shared -L$(MATLAB_DIR)/bin/maci64 -lmx -lmex -lmat -lm -lstdc++
	# define extension
	EXT = mexmaci64
endif

MEX_SRC += src/scma_log_mpa.c
MEX_SRC += src/scma_dec_mex.c

CINCLUDE += src

CPPFLAGS += $(addprefix -I,$(CINCLUDE))

OBJECTS = $(addprefix $(OBJ_DIR)/,$(notdir $(MEX_SRC:.c=.o)))

all:   build

build: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: %.c | dir
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

dir:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

clean:
	$(RM) -rf $(OBJ_DIR) $(BIN_DIR)


vpath %.c $(sort $(dir $(MEX_SRC)))

-include $(OBJECTS:.o=.d)
