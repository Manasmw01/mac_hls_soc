/* Copyright 2021 Columbia University SLD Group */

#ifndef __DRIVER_HPP__
#define __DRIVER_HPP__

// Forward declaration
class system_t;

#include "mac_specs.hpp"

typedef axi::axi4_segment<custom_axi::cfg::standard> local_axi;

class driver_t : public sc_module, public local_axi
{
    public:

        // -- Input ports

        // Clock signal
        sc_in<bool> clk;

        // Reset signal
        sc_in<bool> resetn;

        // Interrupt signals
        sc_in<bool> irq;

        // AXI initiator interface to configure and start the accelerator
        w_master<AUTO_PORT>    CCS_INIT_S1(driver_w_initiator);
        r_master<AUTO_PORT>    CCS_INIT_S1(driver_r_initiator);

        // -- References to other modules

        // To call the system functions
        system_t *system_ref;

        // -- Module constructor

        SC_HAS_PROCESS(driver_t);
        driver_t(sc_module_name name)
            : sc_module(name)
            , clk("clk")
            , resetn("resetn")
            , irq("irq")
        {

            SC_THREAD(driver_thread);
            sensitive << clk.pos();
            async_reset_signal_is(resetn, false);

        }

        // -- Processes

        // To handle read and write requests
        void driver_thread(void);

        // -- Functions (read)

        // To read a particular register
        bool do_read(uint32_t addr, uint32_t &data); //TODO Modify data_type

        // -- Functions (write)

        // To write a particular register
        bool do_write(uint32_t addr, uint32_t data);
};

#endif /* __DRIVER_HPP__ */
