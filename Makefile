CC = gcc
CPP = g++
LD = g++
CFLAGS = $(FLAGS)
CPPFLAGS = $(FLAGS)
BIN_NAME = sinuca
RM = rm -f

################################################################################
#
# Some flags that may help during the debug
#
# Electric Fence: -lefence
# GConf: -pg
# Find Extra Errors: -pedantic -Wextra
################################################################################

# ~ ## DEBUG DEEP
# ~ FLAGS =  -Wall -O0 -ggdb  -Werror -lefence
# ~ LDFLAGS = -ggdb  -lefence

# ~ ## DEBUG DEEP FAST
# ~ FLAGS =  -Wall -O2 -ggdb  -Werror -lefence
# ~ LDFLAGS = -ggdb  -lefence

## DEBUG NORMAL
# ~ FLAGS =  -Wall -O1 -ggdb -Werror
# ~ LDFLAGS = -ggdb

# ~ ## EXECUTION
FLAGS =   -O3 -ggdb -Wall -Wextra -Werror
LDFLAGS = -ggdb


########################################################
MACHINE=$(shell uname -m)

ifeq ($(MACHINE),x86_64)
	LIBRARY = -L./libs/64bits -lz -lconfig++
else
	LIBRARY = -L./libs/32bits -lz -lconfig++
endif


SRC_BASIC =			enumerations.cpp \
			 		utils.cpp \
					opcode_package.cpp \
					uop_package.cpp \
					memory_package.cpp

SRC_TRACE_READER = 	trace_reader/trace_reader.cpp

SRC_PROCESSOR =	 	processor/processor.cpp \
					processor/branch_predictor.cpp

SRC_INTERCONNECTION =  	interconnection/interconnection_router.cpp \
						interconnection/interconnection_controller.cpp \
						interconnection/interconnection_interface.cpp

SRC_CACHE_MEMORY =	memory_devices/prefetcher.cpp \
					memory_devices/line_usage_predictor.cpp\
					memory_devices/cache_memory.cpp

SRC_DIRECTORY =		memory_devices/directory_controller.cpp

SRC_MAIN_MEMORY =   memory_devices/memory_controller.cpp

SRC_CORE = sinuca.cpp sinuca_engine.cpp sinuca_configurator.cpp $(SRC_BASIC) $(SRC_TRACE_READER) $(SRC_PROCESSOR) $(SRC_INTERCONNECTION) $(SRC_CACHE_MEMORY) $(SRC_DIRECTORY) $(SRC_MAIN_MEMORY) $(SRC_PARSER)

########################################################

OBJS_CORE_ = ${SRC_CORE:.cpp=.o}
OBJS_CORE = ${OBJS_CORE_:.c=.o}

OBJS = $(OBJS_CORE)

########################################################
# implicit rules

%.o : %.c
	$(CC) -c $(CFLAGS) $< $(LIBRARY) -o $@

%.o : %.cpp
	$(CPP) -c $(CPPFLAGS) $< $(LIBRARY) -o $@

########################################################

all: sinuca
	$(LD) $(LDFLAGS) -o $(BIN_NAME) $(OBJS) $(LIBRARY)
	@echo SiNUCA linked!
	@echo
	@echo Enjoy!

sinuca: $(OBJS_CORE)
	@echo SiNUCA compiled!
	@echo

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BIN_NAME)
	@echo SiNUCA cleaned!
	@echo

