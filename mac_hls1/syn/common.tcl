# Copyright (c) 2011-2022 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# if {[info exists ::env(DMA_WIDTH)] == 0} {
#     puts " ### ERROR: DMA_WIDTH variable is not set!\n-> Please enter DMA_WIDTH=64 for Ariane or DMA_WIDTH=32 for Leon3 before the corresponding make target.  -> Run make $::env(ACCELERATOR)-clean $::env(ACCELERATOR)-distclean before the new attempt."
#     exit
# } else {
set TECH "virtexup"
# set DMA_WIDTH $::env(DMA_WIDTH)

#
# Technology Libraries
#
    if {$TECH eq "virtexup"} {
        set FPGA_PART_NUM "xcvu9p-flga2104-2-e"
        set FPGA_FAMILY "VIRTEX-uplus"
        set FPGA_SPEED_GRADE "-2"
    }

    if {$TECH eq "virtex7"} {
	set FPGA_PART_NUM "xc7vx485tffg1761-2"
        set FPGA_FAMILY "VIRTEX-7"
        set FPGA_SPEED_GRADE "-2"
    }
# }
