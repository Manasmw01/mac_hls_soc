/* Copyright 2021 Columbia University SLD Group */

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <stdlib.h>
#include <string.h>

#include "driver.hpp"
#include "memory.hpp"

#include <mc_scverify.h>

#include "ac_int.h"
#include "ac_fixed.h"
#include "ac_float.h"
#include "mac.hpp"


typedef axi::axi4_segment<custom_axi::cfg::standard> local_axi;

class system_t : public sc_module, public local_axi
{
public:

    // -- Modules

    // Memory's model
    memory_t CCS_INIT_S1(memory);

    // Driver's model
    driver_t *driver;

    CCS_DESIGN(mac) CCS_INIT_S1(acc);

    // -- Input ports

    // Clock signal
    sc_clock clk;

    // Reset signal
    sc_signal<bool> resetn;

    // -- Output ports

    // Interrupt signals
    sc_signal<bool> irq;

    // -- Internal Channels

    local_axi::w_chan<> CCS_INIT_S1(acc_mem_w);
    local_axi::r_chan<> CCS_INIT_S1(acc_mem_r);
    local_axi::w_chan<> CCS_INIT_S1(drive_acc_w);
    local_axi::r_chan<> CCS_INIT_S1(drive_acc_r);

    // -- Constructor

    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : sc_module(name)
        , memory("memory")
        , driver(new driver_t("driver"))
        , clk("clk", 10.00, SC_NS, 0.5, 0, SC_NS, true)
        , resetn("resetn")
        , irq("irq")
        , acc_mem_w("acc_mem_w")
        , acc_mem_r("acc_mem_r")
        , drive_acc_w("drive_acc_w")
        , drive_acc_r("drive_acc_r")
    {

        // Instantiate the accelerator
        sc_object_tracer<sc_clock> trace_clk(clk);

        //Thread to release the Reset
        SC_THREAD(reset);

        // Binding the driver's model
        driver->clk(clk);
        driver->resetn(resetn);
        driver->irq(irq);
        driver->driver_w_initiator(drive_acc_w);
        driver->driver_r_initiator(drive_acc_r);
        driver->system_ref = this;

        // Binding the memory's model
        memory.clk(clk);
        memory.resetn(resetn);
        memory.r_target(acc_mem_r);
        memory.w_target(acc_mem_w);

        // Binding the accelerator
        acc.clk(clk);
        acc.rst(resetn);
        acc.irq(irq);
        acc.w_initiator(acc_mem_w);
        acc.r_initiator(acc_mem_r);
        acc.w_target(drive_acc_w);
        acc.r_target(drive_acc_r);

    }


    void reset() {
        resetn.write(0);
        wait(50, SC_NS);
        resetn.write(1);
    }

    // -- Functions

    // Load the value of the registers
    void load_regs(void);

    // Load the input values in memory
    void load_memory(void);

    // Read the output values from memory
    void dump_memory(void);

    // Verify that the results are correct
    void validate(void);

    // -- Private data

    uint32_t in_data_size;
    uint32_t in_coeff_size;
    uint32_t out_data_size;

    uint32_t in_data_addr;
    uint32_t in_coeff_addr;
    uint32_t out_data_addr;

    uint32_t mem_size;

    int64_t golden_output[NITERS*NVECTORS];
    int64_t output[NITERS*NVECTORS];

};

#endif /* __SYSTEM_HPP__ */
