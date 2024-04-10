/* Copyright 2022 Columbia University SLD Group */

#include "utils.hpp"
#include "driver.hpp"
#include "system.hpp"

// -- Processes
void driver_t::driver_thread(void)
{

    // Reset
    driver_w_initiator.reset();
    driver_r_initiator.reset();

    wait();

    CCS_LOG("\n\n\n === TEST BEGIN ===");

    // Configure

    {
        system_ref->load_memory();

        system_ref->load_regs();

        assert(do_write(offsetof(dma_address_map,cmd_sig), 1));

        wait();
    }

    // Computation

    {
        uint32_t rdata;

        // Wait for the accelerator to finish

        do { wait(); } while (!irq.read());

        // Verify the accelerator status register content and disable the
        // accelerator

        assert(do_read(offsetof(dma_address_map, cmd_sig), rdata));
        assert(rdata == 2);
        assert(do_write(offsetof(dma_address_map,cmd_sig), 0));

        system_ref->dump_memory();
        system_ref->validate();

        CCS_LOG("\n === TEST COMPLETED === \n");

    }

    // Conclude

    {
        sc_stop();
    }
}

// -- Functions (read)

bool driver_t::do_read(uint32_t addr, uint32_t &data)
{

    r_payload r=driver_r_initiator.single_read(addr);

    data=r.data;

    return (r.resp == Enc::XRESP::OKAY);

}

// -- Functions (write)

bool driver_t::do_write(uint32_t addr, uint32_t data)
{

    b_payload b = driver_w_initiator.single_write(addr, data);

    return (b.resp == Enc::XRESP::OKAY);
}
