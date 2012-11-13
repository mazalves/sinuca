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
			 		utils.cpp

SRC_PACKAGE = 		packages/opcode_package.cpp \
					packages/uop_package.cpp \
					packages/memory_package.cpp

SRC_TRACE_READER = 	trace_reader/trace_reader.cpp

SRC_BRANCH_PREDICTOR =	 	branch_predictor/branch_predictor.cpp \
							branch_predictor/branch_predictor_two_level.cpp \
							branch_predictor/branch_predictor_static_taken.cpp \
							branch_predictor/branch_predictor_disable.cpp

SRC_PROCESSOR =	 	processor/processor.cpp \

SRC_INTERCONNECTION =  	interconnection/interconnection_router.cpp \
						interconnection/interconnection_controller.cpp \
						interconnection/interconnection_interface.cpp

SRC_DIRECTORY =		directory/directory_line.cpp\
					directory/directory_controller.cpp

SRC_PREFETCH =		prefetch/prefetcher.cpp \
					prefetch/reference_prediction_line.cpp \
					prefetch/prefetcher_stride.cpp \
					prefetch/prefetcher_disable.cpp

SRC_LINE_USAGE_PREDICTOR =	line_usage_predictor/line_usage_predictor.cpp \
							line_usage_predictor/pht_line.cpp \
							line_usage_predictor/dsbp_metadata_line.cpp \
							line_usage_predictor/line_usage_predictor_dsbp.cpp \
							line_usage_predictor/aht_line.cpp \
							line_usage_predictor/dlec_metadata_line.cpp \
							line_usage_predictor/line_usage_predictor_dlec.cpp \
							line_usage_predictor/line_usage_predictor_lwp.cpp \
							line_usage_predictor/line_usage_predictor_subblock_stats.cpp \
							line_usage_predictor/line_usage_predictor_line_stats.cpp \
							line_usage_predictor/line_usage_predictor_disable.cpp

SRC_CACHE_MEMORY =	cache_memory/cache_memory.cpp

SRC_MAIN_MEMORY =   main_memory/memory_controller.cpp

SRC_CORE = sinuca.cpp sinuca_engine.cpp sinuca_configurator.cpp \
			$(SRC_BASIC) $(SRC_PACKAGE) $(SRC_TRACE_READER) \
			 $(SRC_INTERCONNECTION) \
			 $(SRC_BRANCH_PREDICTOR) $(SRC_PROCESSOR) \
			 $(SRC_DIRECTORY) $(SRC_PREFETCH) $(SRC_LINE_USAGE_PREDICTOR) $(SRC_CACHE_MEMORY) $(SRC_MAIN_MEMORY)

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

