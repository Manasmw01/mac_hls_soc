#Copyright (c) 2011-2022 Columbia University, System Level Design Group
#SPDX-License-Identifier: Apache-2.0

set sfd [file dir [info script]]

# solution new -state initial
options defaults

options set /Input/CppStandard c++11

options set /Input/CompilerFlags {-DCONNECTIONS_ACCURATE_SIM -DCONNECTIONS_NAMING_ORIGINAL -DSEGMENT_BURST_SIZE=16}
options set /Input/SearchPath {.} -append
options set /Input/SearchPath {/opt/matchlib_toolkit/include} -append
options set /Input/SearchPath {/opt/matchlib_toolkit/examples/Boost-Preprocessor-master} -append
options set /Input/SearchPath {../src} -append
options set /Input/SearchPath {/opt/cad/catapult/shared/pkgs/matchlib/cmod/include} -append
options set /Input/SearchPath "$sfd/../src/mem_bank" -append
options set /ComponentLibs/SearchPath "$sfd/../src/mem_bank" -append


options set Input/CompilerFlags -DHLS_CATAPULT
options set Input/CompilerFlags -DHLS_READY

# project new
project new -name mac

flow package require /DesignWrapper
flow package require /SCVerify

flow package require /QuestaSIM
flow package option set /QuestaSIM/ENABLE_CODE_COVERAGE true

#
# Input
#

solution file add "../tb/system.hpp" -exclude true
solution file add "../tb/system.cpp" -exclude true
solution file add "../tb/sc_main.cpp" -exclude true
solution file add "../tb/driver.cpp" -exclude true
solution file add "../tb/driver.hpp" -exclude true
solution file add "../tb/memory.hpp" -exclude true
solution file add "../src/utils.hpp"
solution file add "../src/mac_conf_info.hpp"
solution file add "../src/mac_functions.hpp"
solution file add "../src/mac.hpp"
solution file add "../src/mac.cpp"
solution file add "../src/mem_wrap.hpp"
solution file add "../src/mac_specs.hpp"
solution file add "../src/mac_utils.hpp"


#
# Output
#

go analyze

directive set -DESIGN_HIERARCHY mac

go compile

solution library \
    add mgc_Xilinx-$FPGA_FAMILY$FPGA_SPEED_GRADE\_beh -- \
    -rtlsyntool Vivado \
    -manufacturer Xilinx \
    -family $FPGA_FAMILY \
    -speed $FPGA_SPEED_GRADE \
    -part $FPGA_PART_NUM

solution library add DUAL_PORT_RBW

go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 10.0}}

go assembly
go architect
go allocate
go extract

project save
