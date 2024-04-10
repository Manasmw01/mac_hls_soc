/* Copyright 2022 Columbia University SLD Group */

#include "mac.hpp"

// -- Functions (utils)

#include "mac_utils.hpp"

// -- Functions (kernel)

#include "mac_functions.hpp"

// -- Processes
#define ALIGNMENT_MASK ((DMA_WIDTH / 8) - 1)

void mac::config(void) {

    //Reset the write target interface
    //Reset the Combinational Channels

    w_target.reset();
    conf_info_chan1.ResetWrite();
    conf_info_chan2b.ResetWrite();
    conf_info_chan3b.ResetWrite();

#ifdef HLS_READY
    conf_info_chan3a.ResetWrite();
    conf_info_chan2a.ResetWrite();
#endif

    wait();
    while (1) {
        aw_payload aw;
        w_payload w;
        b_payload b;

        if (w_target.get_single_write(aw, w, b)) {

            const uint32_t wdata = w.data.to_uint();
            const bool aligned = (wdata & ALIGNMENT_MASK) ? false : true;
            const bool is_non_zero = (wdata != 0);

            b.resp = Enc::XRESP::OKAY;
            switch (aw.addr & 0xfff) {

            case offsetof(dma_address_map,in_data_base_addr_sig):
                if (!aligned)
                {
                    CCS_LOG("INVALID ARGUMENT: address is not aligned (w)");
                    b.resp = Enc::XRESP::SLVERR ;
                }
                else
                    cmd.in_data_base_addr_sig = wdata;

                break;
            case offsetof(dma_address_map,in_coeff_base_addr_sig):
                if (!aligned)
                {
                    CCS_LOG("INVALID ARGUMENT: address is not aligned (w)");
                    b.resp = Enc::XRESP::SLVERR;
                }
                else
                    cmd.in_coeff_base_addr_sig = wdata;

                break;
            case offsetof(dma_address_map,out_data_base_addr_sig):
                if (!aligned)
                {
                    CCS_LOG("INVALID ARGUMENT: address is not aligned (w)");
                    b.resp = Enc::XRESP::SLVERR;
                }
                else
                    cmd.out_data_base_addr_sig = wdata;

                break;

            case offsetof(dma_address_map,nsamples_sig):
                if (!aligned)
                {
                    CCS_LOG("INVALID ARGUMENT: address is not aligned (w)");
                    b.resp = Enc::XRESP::SLVERR;
                }
                else
                    cmd.nsamples_sig = wdata;

                break;

            case offsetof(dma_address_map,nvectors_sig):
                if (!aligned)
                {
                    CCS_LOG("INVALID ARGUMENT: address is not aligned (w)");
                    b.resp = Enc::XRESP::SLVERR;
                }
                else
                    cmd.nvectors_sig = wdata;

                break;

            case offsetof(dma_address_map,niters_sig):
                if (!is_non_zero)
                {
                    CCS_LOG("INVALID ARGUMENT: num_w_cols cannot be 0 (w)");
                    b.resp = Enc::XRESP::SLVERR;
                }
                else
                    cmd.niters_sig = wdata;

                break;

            case offsetof(dma_address_map,cmd_sig):
                if (wdata == 0 )
                {
                    //DO_NOTHING
                }
                else if (wdata == 1 )
                {
                    cmd.cmd_sig = wdata;
                    CCS_LOG("\n\nAccelerator configuration:\n ");
                    CCS_LOG("cmd_sig " << cmd.cmd_sig);
                    CCS_LOG("cmd_in_data_base_addr_sig " << cmd.in_data_base_addr_sig);
                    CCS_LOG("cmd_in_coeff_base_addr_sig " << cmd.in_coeff_base_addr_sig);
                    CCS_LOG("cmd_out_data_base_addr_sig " << cmd.out_data_base_addr_sig);
                    CCS_LOG("cmd_nsamples_sig " << cmd.nsamples_sig);
                    CCS_LOG("cmd_nvectors_sig " << cmd.nvectors_sig);
                    CCS_LOG("cmd_niters_sig " << cmd.niters_sig << "\n");

                    //Send the configuration information to other threads
                    conf_info_chan1.Push(cmd);
                    conf_info_chan2b.Push(cmd);
                    conf_info_chan3b.Push(cmd);
#ifdef HLS_READY
                    conf_info_chan2a.Push(cmd);
                    conf_info_chan3a.Push(cmd);
#endif

                }
                else
                {
                    CCS_LOG("INVALID COMMAND (w)");
                    b.resp = Enc::XRESP::SLVERR;
                }

                break;

            default:
                CCS_LOG("REGISTER NOT MAPPED (w)");
                b.resp = Enc::XRESP::SLVERR;
                break;

            }
            w_target.b.Push(b);

        }
    }
}

void mac::done_read(void) {

    //Reset the read target interface

    r_target.reset();
    wait();

    while (1) {

        ar_payload ar;
        r_payload r;
        bool d;

        if (r_target.single_read(ar, r)) {

            switch (ar.addr & 0xfff) {
            case offsetof(dma_address_map, cmd_sig):
                d=irq.read();
                if (d==true)
                    r.data = 2 ;
                else
                    r.data = 0 ;
                break;

            default:
                CCS_LOG("REGISTER NOT MAPPED");
                r.resp = Enc::XRESP::SLVERR;
                break;
            }

            r_target.r.Push(r); // ???
        }
    }
}


void mac::load_input(void)
{
    bool pingpong=true; 

    //Reset the AXI segment read interface
    //Reset the Combinational Channels and SyncChannels

    AXI4_R_SEGMENT_RESET(r_segment0, r_initiator);
    conf_info_chan1.ResetRead();
    sync12b.reset_sync_out();

#ifdef HLS_READY
    sync12a.reset_sync_out();
    in_ping_w.ResetWrite();
    in_pong_w.ResetWrite();
    in_ping_coeff_w.ResetWrite();
    in_pong_coeff_w.ResetWrite();
#endif

    wait();

    while (true)
    {
        // Retrieve configuration information from config process
        conf_info cmd = conf_info_chan1.Pop();

        uint32_t in_data_base_addr    = cmd.in_data_base_addr_sig;
        uint32_t in_coeff_base_addr  = cmd.in_coeff_base_addr_sig;
        uint32_t out_data_base_addr = cmd.out_data_base_addr_sig;
        uint16_t nsamples        = cmd.nsamples_sig;
        uint32_t nvectors        = cmd.nvectors_sig;
        uint32_t niters        = cmd.niters_sig;

        // Load

            for (uint32_t i = 0; i < niters; i++) {

                // Load
                load_data(i,in_data_base_addr, in_coeff_base_addr, nsamples, nvectors, pingpong);
# ifdef HLS_READY
                // Handshake with compute_req
                sync12a.sync_out();
#endif
                // Handshake with compute
                sync12b.sync_out();

                // Update pingpong var
                pingpong = !pingpong;
            }
    }
}

#ifdef HLS_READY
void mac::compute_kernel_req(void)
{
    bool pingpong=true;

    //Reset the Combinational Channels and SyncChannels

    conf_info_chan2a.ResetRead();
    sync12a.reset_sync_in();
    sync2a3a.reset_sync_out();
    sync2a3b.reset_sync_out();

    in_ping_ra.ResetWrite();
    in_pong_ra.ResetWrite();
    in_ping_coeff_ra.ResetWrite();
    in_pong_coeff_ra.ResetWrite();

    wait();

    while (true)
    {

        conf_info cmd = conf_info_chan2a.Pop();

        uint32_t niters        = cmd.niters_sig;
        uint16_t nsamples      = cmd.nsamples_sig;
        uint32_t nvectors      = cmd.nvectors_sig;

        // Compute_req

        for (uint32_t j = 0; j < niters ; j++)
        {
            // Handshake with load_input
            sync12a.sync_in();

            // Send requests to the input and input coeff ping-pong buffers
            do_mac_req(pingpong,nsamples,nvectors);

            // Update pingpong var
            pingpong = !pingpong;

            // Handshake with store_output_req
            sync2a3a.sync_out();

            // Handshake with store_output
            sync2a3b.sync_out();
        }
    }
}
#endif

void mac::compute_kernel(void)
{
    bool pingpong=true;

    //Reset the Combinational Channels and SyncChannels

    conf_info_chan2b.ResetRead();

    sync12b.reset_sync_in();
    sync2b3b.reset_sync_out();


#ifdef HLS_READY

    sync2b3a.reset_sync_out();
    in_ping_rd.ResetRead();
    in_pong_rd.ResetRead();
    in_ping_coeff_rd.ResetRead();
    in_pong_coeff_rd.ResetRead();

    accumulation_ping_w.ResetWrite();
    accumulation_pong_w.ResetWrite();
#endif

    wait();

    while (true)
    {

        conf_info cmd = conf_info_chan2b.Pop();

        uint32_t niters        = cmd.niters_sig;
        uint16_t nsamples      = cmd.nsamples_sig;
        uint16_t nvectors      = cmd.nvectors_sig;

        // Compute

        for (uint32_t j = 0; j < niters ; j++)
        {
            // Handshake with load_input
            sync12b.sync_in();

            // Execute the computational kernel
            do_mac(pingpong,nsamples,nvectors);

            // Update pingpong var
            pingpong = !pingpong;

#ifdef HLS_READY
            // Handshake with store_output_req
            sync2b3a.sync_out();
#endif
            // Handshake with store_output
            sync2b3b.sync_out();

        }
    }
}


#ifdef HLS_READY

void mac::store_output_req(void)
{

    bool pingpong=true;

    //Reset the Combinational Channels and SyncChannels

    sync2b3a.reset_sync_in();
    conf_info_chan3a.ResetRead();
    sync2a3a.reset_sync_in();
    accumulation_ping_ra.ResetWrite();
    accumulation_pong_ra.ResetWrite();

    wait();

    while (true)
    {

        //Access configuration information

        conf_info cmd = conf_info_chan3a.Pop();

        uint32_t niters = cmd.niters_sig;
        uint32_t nvectors = cmd.nvectors_sig;
        uint16_t nsamples = cmd.nsamples_sig;
        uint32_t out_data_base_addr = cmd.out_data_base_addr_sig;


        //Store_req
        for (uint32_t i = 0; i < niters; i++)
        {
            // Handshake with compute_req
            sync2a3a.sync_in();
            // Handshake with compute
            sync2b3a.sync_in();

            // Send read requests to the accumulation ping-pong buffers
            store_data_req(i, out_data_base_addr, nvectors, pingpong);

            // Update pingpong var
            pingpong = !pingpong;

        }

    }

}

#endif


void mac::store_output(void)
{

    bool pingpong=true;

    //Reset the AXI segment write interface
    //Reset the Combinational Channels and SyncChannels
    //Reset the interrupt flag signal

    AXI4_W_SEGMENT_RESET(w_segment0, w_initiator);
    sync2b3b.reset_sync_in();
    conf_info_chan3b.ResetRead();

#ifdef HLS_READY
    sync2a3b.reset_sync_in();
    accumulation_ping_rd.ResetRead();
    accumulation_pong_rd.ResetRead();
#endif
    irq.write(false);

    wait();

    while (true)
    {

        conf_info cmd = conf_info_chan3b.Pop();

        uint32_t niters = cmd.niters_sig;
        uint16_t nsamples = cmd.nsamples_sig;
        uint32_t out_data_base_addr = cmd.out_data_base_addr_sig;

        // Store
        for (uint32_t i = 0; i < niters; i++) {
#ifdef HLS_READY
            // Handshake with compute_req
            sync2a3b.sync_in();
#endif
            // Handshake with compute
            sync2b3b.sync_in();

            // Store data
            store_data(i, out_data_base_addr, nsamples, pingpong);

            // Update pingpong var
            pingpong = !pingpong;

        }

        //Raise interrupt
        irq.write(true);
    }
}
