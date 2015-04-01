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

## DEBUG DEEP
# ~ FLAGS =  -Wall -O0 -ggdb  -Werror -lefence
# ~ LDFLAGS = -ggdb  -lefence

## DEBUG PROFILE
# ~ FLAGS =  -Wall -O2 -ggdb  -Werror -pg
# ~ LDFLAGS = -ggdb  -pg

## DEBUG DEEP FAST
# ~ FLAGS = -O2 -ggdb -Wall -Wextra -Werror -lefence
# ~ LDFLAGS = -ggdb  -lefence

## DEBUG NORMAL
# ~ FLAGS =  -Wall -O2 -ggdb -Werror
# ~ LDFLAGS = -ggdb

# ~ ## EXECUTION
FLAGS =   -O3 -ggdb -Wall -Wextra -Werror -std=c++0x
LDFLAGS = -ggdb

########################################################
# ~ MACHINE=$(shell uname -m)

LIBRARY = -L$(CURDIR)/extra_libs/lib -Wl,-rpath,$(CURDIR)/extra_libs/lib -lz -lconfig++

SRC_BASIC =			enumerations.cpp \
			 		utils.cpp

SRC_PACKAGE = 		packages/opcode_package.cpp \
					packages/uop_package.cpp \
					packages/memory_package.cpp

SRC_TRACE_READER = 	trace_reader/trace_reader.cpp

SRC_BRANCH_PREDICTOR =	 	branch_predictor/branch_predictor.cpp \
							branch_predictor/branch_predictor_two_level_gag.cpp \
							branch_predictor/branch_predictor_two_level_gas.cpp \
							branch_predictor/branch_predictor_two_level_pag.cpp \
							branch_predictor/branch_predictor_two_level_pas.cpp \
							branch_predictor/branch_predictor_bi_modal.cpp \
							branch_predictor/branch_predictor_static_taken.cpp \
							branch_predictor/branch_predictor_perfect.cpp \
							branch_predictor/branch_predictor_disable.cpp

SRC_PROCESSOR =	 	processor/processor.cpp \
					processor/reorder_buffer_line.cpp \
					processor/memory_order_buffer_line.cpp

SRC_INTERCONNECTION =	interconnection/token.cpp \
						interconnection/interconnection_router.cpp \
						interconnection/interconnection_controller.cpp \
						interconnection/interconnection_interface.cpp

SRC_DIRECTORY =		directory/directory_line.cpp\
					directory/directory_controller.cpp

SRC_PREFETCH =		prefetch/prefetcher.cpp \
					prefetch/stride_table_line.cpp \
					prefetch/prefetcher_stride.cpp \
					prefetch/stream_table_line.cpp \
					prefetch/prefetcher_stream.cpp \
					prefetch/prefetcher_disable.cpp

SRC_LINE_USAGE_PREDICTOR =	line_usage_predictor/line_usage_predictor.cpp \
							line_usage_predictor/line_usage_predictor_disable.cpp \
							line_usage_predictor/pht_line.cpp \
							line_usage_predictor/dsbp_metadata_line.cpp \
							line_usage_predictor/line_usage_predictor_dsbp.cpp \
							line_usage_predictor/line_usage_predictor_dsbp_oracle.cpp \
							line_usage_predictor/aht_line.cpp \
							line_usage_predictor/dewp_metadata_line.cpp \
							line_usage_predictor/line_usage_predictor_dewp.cpp \
							line_usage_predictor/line_usage_predictor_dewp_oracle.cpp\
							line_usage_predictor/skewed_metadata_line.cpp \
							line_usage_predictor/line_usage_predictor_skewed.cpp


SRC_CACHE_MEMORY =	cache_memory/mshr_diff_line.cpp \
					cache_memory/cache_memory.cpp

SRC_MAIN_MEMORY =   main_memory/memory_channel.cpp \
					main_memory/memory_controller.cpp

SRC_CORE =  sinuca.cpp sinuca_engine.cpp sinuca_configurator.cpp \
			$(SRC_BASIC) $(SRC_PACKAGE) $(SRC_TRACE_READER) \
			 $(SRC_INTERCONNECTION) \
			 $(SRC_BRANCH_PREDICTOR) $(SRC_PROCESSOR) \
			 $(SRC_DIRECTORY) $(SRC_PREFETCH) $(SRC_LINE_USAGE_PREDICTOR) $(SRC_CACHE_MEMORY) $(SRC_MAIN_MEMORY)

########################################################
OBJS_CORE = ${SRC_CORE:.cpp=.o}
OBJS = $(OBJS_CORE)
########################################################
# implicit rules
%.o : %.cpp %.hpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

########################################################

all: sinuca

sinuca: extra_libs/lib/libconfig++.a  extra_libs/lib/libz.a $(OBJS_CORE)
	$(LD) $(LDFLAGS) -o $(BIN_NAME) $(OBJS) $(LIBRARY)

extra_libs/lib/libconfig++.a:
	@mkdir -p extra_libs/lib
	@echo Building libconfig
	cd extra_libs/src/; \
	tar xzf libconfig-*.tar.gz; \
	cd libconfig-*; \
	./configure --prefix=$(CURDIR)/extra_libs/; \
	make; \
	make install

extra_libs/lib/libz.a:
	@mkdir -p extra_libs/lib
	@echo Building zlib
	cd extra_libs/src/; \
	tar xzf zlib-*.tar.gz; \
	cd zlib-*; \
	./configure --prefix=$(CURDIR)/extra_libs/; \
	make; \
	make install

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BIN_NAME)
	@echo SiNUCA cleaned!
	@echo

distclean: clean
	-$(RM) extra_libs/lib/* -R
	-$(RM) extra_libs/include/* -R
	-$(RM) extra_libs/share/* -R
	@echo Cleaning libconfig
	cd extra_libs/src/libconfig-*; \
	make distclean
	@echo Cleaning zlib
	cd extra_libs/src/zlib-*; \
	make distclean
	@echo SiNUCA cleaned!
	@echo
