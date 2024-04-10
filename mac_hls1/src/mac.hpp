#pragma once

#include "mac_specs.hpp"

#include "utils.hpp"
#include "mem_wrap.hpp"

#ifndef __MAC_HPP__
#define __MAC_HPP__


typedef axi::axi4_segment<custom_axi::cfg::standard> local_axi;

#pragma hls_design top
class mac
    : public sc_module
    , public local_axi
{
public:

    // -- Input ports

    // Clock signal
    sc_in<bool> clk;

    // Reset signal
    sc_in<bool> rst;

    // -- Output ports

    // Interrupt signal
    sc_out<bool> irq;

    // -- AXI interfaces:
    //AXI4 initiator interface with memory
    r_master<> CCS_INIT_S1(r_initiator);
    w_master<> CCS_INIT_S1(w_initiator);
    //AXI4 target interface with driver
    w_slave<>  CCS_INIT_S1(w_target);
    r_slave<>  CCS_INIT_S1(r_target);

    // -- Constructor

    SC_CTOR(mac)
#ifdef HLS_READY
        :  input_ping("input_ping"),
           input_pong("input_ping"),
           input_coeff_ping("input_coeff_ping"),
           input_coeff_pong("input_coeff_pong")
#endif
    {
        //Bind the AXI-segments to the initiator AXI interfaces -> see
        //$(MATCHLIB_TOOL)/include/axi4_segment.h
        AXI4_W_SEGMENT_BIND(w_segment0, clk, rst, w_initiator);
        AXI4_R_SEGMENT_BIND(r_segment0, clk, rst, r_initiator);

        //Register Processes

        SC_THREAD(config);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

        SC_THREAD(done_read);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

        SC_THREAD(load_input);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

        SC_THREAD(compute_kernel);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

        SC_THREAD(store_output);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

#ifdef HLS_READY
        SC_THREAD(compute_kernel_req);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

        SC_THREAD(store_output_req);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);
#endif

#ifdef HLS_READY

        //Bind scratchpad wrappers' inerfaces

        input_ping.clk(clk);
        input_ping.rst(rst);
        input_ping.write_req(in_ping_w);
        input_ping.read_req(in_ping_ra);
        input_ping.read_rsp(in_ping_rd);

        input_pong.clk(clk);
        input_pong.rst(rst);
        input_pong.write_req(in_pong_w);
        input_pong.read_req(in_pong_ra);
        input_pong.read_rsp(in_pong_rd);

        input_coeff_ping.clk(clk);
        input_coeff_ping.rst(rst);
        input_coeff_ping.write_req(in_ping_coeff_w);
        input_coeff_ping.read_req(in_ping_coeff_ra);
        input_coeff_ping.read_rsp(in_ping_coeff_rd);

        input_coeff_pong.clk(clk);
        input_coeff_pong.rst(rst);
        input_coeff_pong.write_req(in_pong_coeff_w);
        input_coeff_pong.read_req(in_pong_coeff_ra);
        input_coeff_pong.read_rsp(in_pong_coeff_rd);

        accumulation_ping.clk(clk);
        accumulation_ping.rst(rst);
        accumulation_ping.write_req(accumulation_ping_w);
        accumulation_ping.read_req(accumulation_ping_ra);
        accumulation_ping.read_rsp(accumulation_ping_rd);

        accumulation_pong.clk(clk);
        accumulation_pong.rst(rst);
        accumulation_pong.write_req(accumulation_pong_w);
        accumulation_pong.read_req(accumulation_pong_ra);
        accumulation_pong.read_rsp(accumulation_pong_rd);

#endif

    }

private:

    // -- Processes
    //Get CSRs configuration through the AXI-write target interface
    void config(void);

    //Expose the accelerator status register through the AXI-read target
    //interface
    void done_read(void);

    // Load input from memory through the AXI-read initiator interface
    void load_input(void);

    // Perform the computation
    void compute_kernel(void);

#ifdef HLS_READY
    // Send Read Requests to the local scratchpad to perform the computation
    void compute_kernel_req(void);

    // Send Read Requests to the local scratchpad to store the output in memory
    void store_output_req(void);
#endif

    // Store output in memory through the AXI-write initiator interface
    void store_output(void);

    // -- Functions (utils)
    // Kernel
    inline void do_mac(bool ping_pong, uint16_t nsamples, uint32_t nvectors);

#ifdef HLS_READY
    // Send read requests to local scratchpad for Kernel Computation
    inline void do_mac_req(bool ping_pong, uint16_t nsamples, uint32_t nvectors);

    // Send read requests to local scratchpad for store
    inline void store_data_req(uint32_t iter, uint32_t out_base_addr, uint32_t nvectors, bool ping_pong);
#endif

    // To read the input data
    inline void load_data(uint32_t iter, uint32_t base_addr, uint32_t base_addr_coeff, uint16_t nsamples,uint32_t nvectors, bool ping_pong);

    // To store the output data
    inline void store_data(uint32_t iter, uint32_t out_base_addr, uint32_t nvectors, bool ping_pong);

    //Other private members:

    // -- SyncChannel for Handshakes across threads:

    //Handshake load-compute
    Connections::SyncChannel CCS_INIT_S1(sync12b);
    //Handshake compute-store
    Connections::SyncChannel CCS_INIT_S1(sync2b3b);


#ifdef HLS_READY
    //Handshake load-compute_req
    Connections::SyncChannel CCS_INIT_S1(sync12a);
    //Handshake compute_req-store_req
    Connections::SyncChannel CCS_INIT_S1(sync2a3a);
    //Handshake compute-store_req
    Connections::SyncChannel CCS_INIT_S1(sync2b3a);
    //Handshake compute_req-store
    Connections::SyncChannel CCS_INIT_S1(sync2a3b);
#endif

    // -- AXI4 segments -> see
    //$(MATCHLIB_TOOL)/include/axi4_segment.h
    AXI4_W_SEGMENT(w_segment0)
    AXI4_R_SEGMENT(r_segment0)


    // -- CSRs for configuration information
    conf_info cmd;

    // Combinational Channels for sharing configuration information across
    // threads
    Connections::Combinational<conf_info> CCS_INIT_S1(conf_info_chan1);
    Connections::Combinational<conf_info> CCS_INIT_S1(conf_info_chan2b);
    Connections::Combinational<conf_info> CCS_INIT_S1(conf_info_chan3b);
#ifdef HLS_READY
    Connections::Combinational<conf_info> CCS_INIT_S1(conf_info_chan2a);
    Connections::Combinational<conf_info> CCS_INIT_S1(conf_info_chan3a);
#endif

// -- Private memories - Matchlib Scratchpads Wrappers -> see mem_wrap.hpp
#ifdef HLS_READY
    //Memory wrappers instantiations: PING-PONG scratchpads for both the input data
    // and the coefficients
    mem_wrap<inbks, inrp,
             inwp, inebks,
             DATA_TYPE32, NVUINTW(in_as),
             plm_WR<in_as, inwp,32>,
             plm_RRq<in_as,inrp>,
             plm_RRs<inrp,32>> CCS_INIT_S1(input_ping);

    mem_wrap<inbks, inrp,
             inwp, inebks,
             DATA_TYPE32, NVUINTW(in_as),
             plm_WR<in_as, inwp,32>,
             plm_RRq<in_as,inrp>,
             plm_RRs<inrp,32>> CCS_INIT_S1(input_pong);

    mem_wrap<incbks, incrp,
             incwp, incebks,
             DATA_TYPE16, NVUINTW(inc_as),
             plm_WR<inc_as, incwp,16>,
             plm_RRq<inc_as,incrp>,
             plm_RRs<incrp,16>> CCS_INIT_S1(input_coeff_ping);

    mem_wrap<incbks, incrp,
             incwp, incebks,
             DATA_TYPE16, NVUINTW(inc_as),
             plm_WR<inc_as, incwp,16>,
             plm_RRq<inc_as,incrp>,
             plm_RRs<incrp,16>> CCS_INIT_S1(input_coeff_pong);

    mem_wrap<outbks, outrp,
             outwp, outebks,
             DATA_TYPE16, NVUINTW(out_as),
             plm_WR<out_as, outwp,64>,
             plm_RRq<out_as,outrp>,
             plm_RRs<outrp,64>> CCS_INIT_S1(accumulation_ping);

    mem_wrap<outbks, outrp,
             outwp, outebks,
             DATA_TYPE16, NVUINTW(out_as),
             plm_WR<out_as, outwp,64>,
             plm_RRq<out_as,outrp>,
             plm_RRs<outrp,64>> CCS_INIT_S1(accumulation_pong);

    //Combinational Channels used to access the memory wrappers:
    // *_w:write request channel;
    // *_ra:read request channel;
    // *_rd:read response channel;
    Connections::Combinational<plm_WR<in_as,inwp,32>> in_ping_w;
    Connections::Combinational<plm_RRq<in_as,inrp>> in_ping_ra;
    Connections::Combinational<plm_RRs<inrp,32>> in_ping_rd;

    Connections::Combinational<plm_WR<in_as,inwp,32>> in_pong_w;
    Connections::Combinational<plm_RRq<in_as,inrp>> in_pong_ra;
    Connections::Combinational<plm_RRs<inrp,32>> in_pong_rd;

    Connections::Combinational<plm_WR<inc_as,incwp,16>> in_ping_coeff_w;
    Connections::Combinational<plm_RRq<inc_as,incrp>> in_ping_coeff_ra;
    Connections::Combinational<plm_RRs<incrp,16>> in_ping_coeff_rd;

    Connections::Combinational<plm_WR<inc_as,incwp,16>> in_pong_coeff_w;
    Connections::Combinational<plm_RRq<inc_as,incrp>> in_pong_coeff_ra;
    Connections::Combinational<plm_RRs<incrp,16>> in_pong_coeff_rd;

    Connections::Combinational<plm_WR<out_as,outwp,64>> accumulation_ping_w;
    Connections::Combinational<plm_RRq<out_as,outrp>> accumulation_ping_ra;
    Connections::Combinational<plm_RRs<outrp,64>> accumulation_ping_rd;

    Connections::Combinational<plm_WR<out_as,outwp,64>> accumulation_pong_w;
    Connections::Combinational<plm_RRq<out_as,outrp>> accumulation_pong_ra;
    Connections::Combinational<plm_RRs<outrp,64>> accumulation_pong_rd;

#else
    //Not synthesizable pin-pong arrays
    int32_t in_data_buf[2][NSAMPLES*NVECTORS];
    int16_t in_coeff_buf[2][NSAMPLES*NVECTORS];
    int64_t accumulation[2][NVECTORS];
#endif

};

#endif /* __MAC_HPP__ */
