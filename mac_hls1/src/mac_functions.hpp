/* Copyright 2021 Columbia University SLD Group */

#ifndef __MAC_FUNCTIONS_HPP__
#define __MAC_FUNCTIONS_HPP__

#include "mac.hpp"

#ifdef HLS_READY
void mac::do_mac_req(bool ping_pong, uint16_t nsamples, uint32_t nvectors)
{
    const uint32_t plm_id = ping_pong ? 1 : 0;

    //Push Read Requests to the input and input_coeff
    //scratchpads to retrieve data for computation
    for (uint32_t i = 0; i < nsamples*nvectors; i++) {

        plm_RRq<in_as,inrp> rreq;
        rreq.indx[0]=i;
        rreq.val[0]=true;

        plm_RRq<inc_as,incrp> rreq1;
        rreq1.indx[0]=i;
        rreq1.val[0]=true;

        if (ping_pong)
        {
            in_ping_ra.Push(rreq);
            in_ping_coeff_ra.Push(rreq1);

        }
        else
        {
            in_pong_ra.Push(rreq);
            in_pong_coeff_ra.Push(rreq1);
        }
    }
}
#endif

void mac::do_mac(bool ping_pong, uint16_t nsamples, uint32_t nvectors)
{
    const uint32_t plm_id = ping_pong ? 1 : 0;
    uint32_t out_index=0 ;

#ifdef HLS_READY
    FPDATA_WORD64 tmpMult=0;
#else
    int64_t tmpMult=0;
#endif
    int count=0;
    for (uint32_t i = 0; i < nsamples * nvectors; i++) {
#ifdef HLS_READY

        //Pop Read Responses from input and input_coeff scratchpads
        //and compute

        FPDATA_WORD32 data;
        FPDATA_WORD16 coeff;

        if (ping_pong)
        {
            data=in_ping_rd.Pop().data[0];
            coeff=in_ping_coeff_rd.Pop().data[0];

        }
        else
        {
            data=in_pong_rd.Pop().data[0];
            coeff=in_pong_coeff_rd.Pop().data[0];
        }

        tmpMult+=data*coeff;
        count+=1;

        //Once done with the accumulation from the first vector of the
        //iteration,push the results to the store_data
        //thread using the dedicated connections
        if(count==nsamples)
        {
            plm_WR<out_as,outwp,64> wreq;
            wreq.data[0]=tmpMult;
            wreq.indx[0]=out_index++;

            if (ping_pong)
                accumulation_ping_w.Push(wreq);
            else
                accumulation_pong_w.Push(wreq);

            tmpMult=0;
            count=0;
        }

#else
        tmpMult +=
            (int64_t) in_data_buf[plm_id][i] *
            (int64_t) in_coeff_buf[plm_id][i];

        if((i+1)%nsamples==0 && i>0)
        {
            accumulation[plm_id][out_index++]=tmpMult;
            tmpMult=0;
        }
#endif
    }

}

#endif /* __MAC_FUNCTIONS_HPP__ */
