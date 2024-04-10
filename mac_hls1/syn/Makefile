# Copyright (c) 2011-2022 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

CXXFLAGS += -g -std=c++11 -Wall -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-label -Wno-maybe-uninitialized

ifndef SYSTEMC_HOME
$(error - Environment variable SYSTEMC_HOME must be defined)
endif
ifndef CONNECTIONS_HOME
$(error - Environment variable CONNECTIONS_HOME must be defined)
endif
ifndef MATCHLIB_HOME
$(error - Environment variable MATCHLIB_HOME must be defined)
endif
ifndef BOOST_HOME
$(error - Environment variable BOOST_HOME must be defined)
endif
ifndef RAPIDJSON_HOME
$(error - Environment variable RAPIDJSON_HOME must be defined)
endif
ifndef AC_TYPES
$(error - Environment variable AC_TYPES must be defined)
endif
ifndef AC_SIMUTILS
$(error - Environment variable AC_SIMUTILS must be defined)
endif

# Default to the compiler installed on the machine
CXX ?= g++
LD_LIBRARY_PATH := $(if $(LD_LIBRARY_PATH),$(LD_LIBRARY_PATH):$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64,$(SYSTEMC_HOME)/lib:$(SYSTEMC_HOME)/lib-linux64)
export LD_LIBRARY_PATH
LIBDIRS += -L$(SYSTEMC_HOME)/lib -L$(SYSTEMC_HOME)/lib-linux64

# endif

# ---------------------------------------------------------------------

# Check: $(SYSTEMC_HOME)/include/systemc.h must exist
checkvar_SYSTEMC_HOME: $(SYSTEMC_HOME)/include/systemc.h

# Check: $(CONNECTIONS_HOME)/include/connections/connections.h must exist
checkvar_CONNECTIONS_HOME: $(CONNECTIONS_HOME)/include/connections/connections.h

# Check: $(MATCHLIB_HOME)/cmod/include/nvhls_marshaller.h
checkvar_MATCHLIB_HOME: $(MATCHLIB_HOME)/cmod/include/nvhls_marshaller.h

# Check: $(BOOST_HOME)/include/boost/preprocessor/arithmetic/add.hpp
checkvar_BOOST_HOME: $(BOOST_HOME)/include/boost/preprocessor/arithmetic/add.hpp

# Check: $(RAPIDJSON_HOME)/include/rapidjson/document.h
checkvar_RAPIDJSON_HOME: $(RAPIDJSON_HOME)/include/rapidjson/document.h

# Check: $(AC_TYPES)/include/ac_int.h
checkvar_AC_TYPES: $(AC_TYPES)/include/ac_int.h

# Check: $(AC_SIMUTILS)/include/mc_scverify.h
checkvar_AC_SIMUTILS: $(AC_SIMUTILS)/include/mc_scverify.h

# Rule to check that environment variables are set correctly
checkvars: checkvar_SYSTEMC_HOME checkvar_CONNECTIONS_HOME checkvar_MATCHLIB_HOME checkvar_BOOST_HOME checkvar_RAPIDJSON_HOME checkvar_AC_TYPES checkvar_AC_SIMUTILS
# =====================================================================

export CATAPULT_HOME SYSTEMC_HOME CONNECTIONS_HOME MATCHLIB_HOME BOOST_HOME RAPIDJSON_HOME AC_TYPES AC_SIMUTILS

# Determine the director containing the source files from the path to this Makefile
PWD := $(shell pwd)
SOURCE_DIR = $(subst syn,src/,$(PWD))
TB_DIR = $(subst syn,tb/,$(PWD))

SRCS := $(wildcard $(SOURCE_DIR)*.cpp)
SRCS += $(wildcard $(TB_DIR)*.cpp)

INCDIRS += -I$(SOURCE_DIR) -I$(TB_DIR) -I$(PWD)/../../pv/mojo -I/opt/opencv/include

INCDIRS += -I$(SYSTEMC_HOME)/include -I$(SYSTEMC_HOME)/src
INCDIRS += -I$(MATCHLIB_TOOL)/matchlib_examples/include
INCDIRS += -I$(CONNECTIONS_HOME)/include
INCDIRS += -I$(MATCHLIB_HOME)/cmod/include
INCDIRS += -I$(BOOST_HOME)/include
INCDIRS += -I$(RAPIDJSON_HOME)/include
INCDIRS += -I$(AC_TYPES)/include
INCDIRS += -I$(AC_MATH)/include
INCDIRS += -I$(AC_SIMUTILS)/include


CPPFLAGS += $(INCDIRS)
CPPFLAGS += -DCONNECTIONS_ACCURATE_SIM -DSC_INCLUDE_DYNAMIC_PROCESSES
CPPFLAGS += -DSEGMENT_BURST_SIZE=16
CXXFLAGS += -D__CUSTOM_SIM__
CXXFLAGS += -D__MATCHLIB_CONNECTIONS__
CXXFLAGS += -DHLS_CATAPULT

LIBS += -lsystemc -lpthread

CATAPULT_PRODUCT = ultra

.PHONY: all build run clean sim_clean hls
build: sim_sc

all: run hls

$(INPUT_PATH)/preprocess:
	@$(MAKE) -C $(INPUT_PATH)

$(BINS): $(INPUT_PATH)/preprocess
	@ln -s $(INPUT_PATH)/$@

sim_sc: $(SRCS) $(BINS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

sim_sc_syn: $(SRCS) $(BINS)
	$(CXX) $(CXXFLAGS) -DHLS_READY $(CPPFLAGS) $(LIBDIRS) $(SRCS) -o $@ $(LIBS)

#This target is used to run behavioral SystemC simulation
trace.vcd: sim_sc_syn
	-@echo "Starting execution in directory `pwd`"
	./$^

run_syn: trace.vcd

run: sim_sc
	-@echo "Starting execution in directory `pwd`"
	./$^


# These two targets assume that the QuestaSim utilities 'vcd2wlf' and 'vsim'
# are found in your PATH
%.wlf: %.vcd
	vcd2wlf $< $@

view_wave_syn: trace.wlf
	vsim $< -nolog -do "add wave -r trace:/SystemC/*" -do "wave zoom full"

#This target is used to run HLS
hls:
	@echo '   ' ;if test -e ./mac; then \
		rm -r mac; \
	fi;
	catapult -product $(CATAPULT_PRODUCT) -shell -f ./build_prj_top.tcl &

hls-gui:
	@echo '   ' ;if test -e ./mac; then \
		rm -r mac; \
	fi;
	catapult -product $(CATAPULT_PRODUCT) -f ./build_prj_top.tcl &

#These targets are used to run RTL-simulation
sim:
	@echo '   ' ;if test -e ./mac; then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim.tcl & \
	else \
		echo "Run HLS before RTL-simulation"; \
        fi;

sim-gui:
	@echo '   ' ;if test -e ./mac; then \
		catapult -product $(CATAPULT_PRODUCT) -shell -f ./rtl_sim_gui.tcl & \
	else \
		echo "Run HLS before RTL-simulation"; \
        fi;


# These two targets assume that the QuestaSim utilities 'vcd2wlf' and 'vsim'
# are found in your PATH
sim_clean:
	@rm -rf sim_* *_data.txt *_names.txt

help: checkvars
	-@echo "Makefile targets:"
	-@echo "  clean     - Clean up from previous make runs"
	-@echo "  all       - Perform all of the targets below"
	-@echo "  sim_sc    - Compile SystemC design"
	-@echo "  run       - Execute SystemC design and generate trace.vcd"
	-@echo "  view_wave - Convert trace.vcd to QuestaSim wlf file and view in QuestaSim"
	-@echo ""
	-@echo "Environment/Makefile Variables:"
	-@echo "  CATAPULT_HOME      = $(CATAPULT_HOME)"
	-@echo "  SYSTEMC_HOME       = $(SYSTEMC_HOME)"
	-@echo "  CONNECTIONS_HOME   = $(CONNECTIONS_HOME)"
	-@echo "  MATCHLIB_HOME      = $(MATCHLIB_HOME)"
	-@echo "  BOOST_HOME         = $(BOOST_HOME)"
	-@echo "  RAPIDJSON_HOME     = $(RAPIDJSON_HOME)"
	-@echo "  AC_TYPES           = $(AC_TYPES)"
	-@echo "  AC_SIMUTILS        = $(AC_SIMUTILS)"
	-@echo "  CXX                = $(CXX)"
	-@echo "  LIBDIRS            = $(LIBDIRS)"
	-@echo "  LD_LIBRARY_PATH    = $(LD_LIBRARY_PATH)"
	-@echo ""

clean: sim_clean
	@rm -rf *.vcd *.wlf *atap* wrapDir *.bin



