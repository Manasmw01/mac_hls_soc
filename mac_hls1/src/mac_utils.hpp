/* Copyright 2021 Columbia University SLD Group */

// #include "register_map.hpp"

#ifndef __MAC_UTILS_HPP__
#define __MAC_UTILS_HPP__

#include "mac.hpp"

#define AXI4_BURST_MAX_LEN 16

void mac::load_data(uint32_t iter, uint32_t base_addr, uint32_t base_addr_coeff, uint16_t nsamples, uint32_t nvectors, bool ping_pong)
{
    const uint32_t plm_id = ping_pong ? 1 : 0;

    // Load in_data
    {
        const uint32_t dma_adj = DMA_WIDTH / 32; //Adjustment factor
        const uint32_t size = nsamples *nvectors/ dma_adj;  //Total transaction length
        const uint32_t iter_base_addr = iter * nsamples * nvectors* 4;//Offset
                                                                      //for AXI
                                                                      //transaction
                                                                      //memory address
        uint32_t index = 0;
        uint32_t beat_id = 1;
        uint32_t plm_index = 0;
        uint32_t mem_off = base_addr + iter_base_addr;

        //AXI4 Read request type
        ex_ar_payload ar;

        for (index = size; index > 0; )
        {

            uint32_t j = 0;
            uint32_t beats = (index < 16) ? index : 16;

            //Set AXI read request parameters
            ar.id = beat_id;
            ar.ex_len = beats - 1;
            ar.addr = mem_off;

            //Push the AXI read request to the AXI segment
            r_segment0_ex_ar_chan.Push(ar);

            for (uint32_t j = 0; j < beats; ++j)
            {
                //Pop the AXI read response from the initiator read interface
                r_payload r = r_initiator.r.Pop();

                for (uint32_t k = 0; k < dma_adj; k++) {
                    const uint32_t shift_factor = 32 * k;
#ifdef HLS_READY
                    //Push Write requests to the input scratchpads through the
                    //dedicated Combinational channels
                    plm_WR<in_as,inwp,32> wreq;
                    wreq.data[0]=0xffffffff & (r.data >> shift_factor);
                    wreq.indx[0]=plm_index + k;

                    if (ping_pong)
                        in_ping_w.Push(wreq);
                    else
                        in_pong_w.Push(wreq);

#else
                    in_data_buf[plm_id][plm_index + k] = 0xffffffff & (r.data >> shift_factor);

#endif
                }

                plm_index += dma_adj;
            }

            mem_off += beats * (DMA_WIDTH / 8);
            index -= beats;
            ++beat_id;

            wait();
        }
    }

    // Load in_coeff (same as above, for coefficients)
    {
        const uint32_t dma_adj = DMA_WIDTH / 16;
        const uint32_t size = nsamples *nvectors/ dma_adj;
        const uint32_t iter_base_addr = iter * nsamples * nvectors * 2;
        uint32_t index = 0;
        uint32_t beat_id = 1;
        uint32_t plm_index = 0;
        uint32_t mem_off = base_addr_coeff + iter_base_addr;

        ex_ar_payload ar;

        for (index = size; index > 0; )
        {
                uint32_t beats = (index < 16) ? index : 16;

                ar.id = beat_id;
                ar.ex_len = beats - 1;
                ar.addr = mem_off;

                r_segment0_ex_ar_chan.Push(ar);

            for (uint32_t j = 0; j < beats; ++j)
            {

                r_payload r = r_initiator.r.Pop();

                for (uint32_t k = 0; k < dma_adj; k++) {
                    const uint32_t shift_factor = 16 * k;
#ifdef HLS_READY
                    plm_WR<inc_as,incwp,16> wreq;
                    wreq.data[0]=0xffffffff & (r.data >> shift_factor);
                    wreq.indx[0]=plm_index + k;

                    if (ping_pong)
                        in_ping_coeff_w.Push(wreq);
                    else
                        in_pong_coeff_w.Push(wreq);
#else
                        in_coeff_buf[plm_id][plm_index + k] = 0xffffffff & (r.data >> shift_factor);

#endif
                }

                plm_index += dma_adj;
            }

            mem_off += beats * (DMA_WIDTH / 8);
            index -= beats;
            ++beat_id;

            wait();
        }
    }
}

#ifdef HLS_READY

void mac::store_data_req(uint32_t iter, uint32_t out_base_addr, uint32_t nvectors, bool ping_pong)
{
    uint32_t size= nvectors;
    uint32_t mem_index = 0;
    uint32_t index = 0;

    //Push Read Requests to the accumulation ping/pong
    //scratchpads to retrieve data for storing it back
    //main memory
    for (index = size; index > 0; )
    {
        uint32_t beats = (index < 16) ? index : 16;

        for (uint32_t j = 0; j < beats; ++j)
        {

            plm_RRq<out_as,outrp> rreq;
            rreq.indx[0]=mem_index;
            rreq.val[0]=true;

            if (ping_pong)
                accumulation_ping_ra.Push(rreq);
            else
                accumulation_pong_ra.Push(rreq);

            mem_index++;
        }

        index -= beats;

    }
}
#endif

void mac::store_data(uint32_t iter, uint32_t out_base_addr, uint32_t nvectors, bool ping_pong)
{

    const uint32_t dma_adj_narrow_bus = 64 / DMA_WIDTH;
    const uint64_t mask_narrow_bus = (((uint64_t) 1) << DMA_WIDTH) - 1;
    const uint32_t iter_base_addr = iter * nvectors * 8;

    int64_t result;

    uint32_t size;
    if (dma_adj_narrow_bus != 0)
        size = dma_adj_narrow_bus;
    else
        size = 1;

    uint32_t index = 0;
    uint32_t beat_id = 1;
    uint32_t mem_off = out_base_addr + iter_base_addr;
    ex_aw_payload aw;

    for (uint32_t i=0; i< nvectors;i++)
    {
#if HLS_READY
        //Pop Read responses from accumulation ping/pong scratchpads
       if (ping_pong)
            result= (int64_t) accumulation_ping_rd.Pop().data[0];
        else
            result= (int64_t) accumulation_pong_rd.Pop().data[0];
#else
        //Read the output from native array
        result = ping_pong ? accumulation[1][i] : accumulation[0][i];
#endif

        //Send data back to main memory through the AXI initiator write interface
        for (index = size; index > 0; )
        {
            uint32_t beats = (index < 16) ? index : 16;

            aw.id = beat_id;
            aw.ex_len = beats - 1;
            aw.addr = mem_off;
            w_segment0_ex_aw_chan.Push(aw);

            for (uint32_t j = 0; j < beats; ++j)
            {
                w_payload w;

                if (dma_adj_narrow_bus != 0) {
                    const uint32_t shift_factor = DMA_WIDTH * j;
                    w.data = (uint32_t) (mask_narrow_bus & (result >> shift_factor));
                } else {
                    w.data = result;
                }

                w.last = (j == beats - 1);

                //Push Write data to main memory
                w_segment0_w_chan.Push(w);

                wait();
            }

            //Pop Write response
            b_payload b = w_segment0_b_chan.Pop();

            //Update variables
            mem_off += (beats  * (DMA_WIDTH / 8));
            index -= beats;
            ++beat_id;

        }

    }
}

#endif // __MAC_UTILS_HPP__
