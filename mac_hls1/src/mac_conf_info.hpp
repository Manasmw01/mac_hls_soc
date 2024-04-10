// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __CONF_INFO_HPP__
#define __CONF_INFO_HPP__

#pragma once

#include <sstream>
#include <ac_int.h>
#include <ac_fixed.h>
#include "mac_specs.hpp"

//
// Configuration parameters for the accelerator.
//

struct conf_info
{

    //Declare struct members

    uint32_t cmd_sig;
    uint32_t in_data_base_addr_sig;
    uint32_t in_coeff_base_addr_sig;
    uint32_t out_data_base_addr_sig;
    uint32_t nsamples_sig;
    uint32_t nvectors_sig;
    uint32_t niters_sig;


    //Declare hardware bitwidth

    static const unsigned int width = 32*7 ;


    //Declare Marshall function to pack/unpack to bits

    template <unsigned int Size> void Marshall(Marshaller <Size> &m) {
        m &cmd_sig;
        m &in_data_base_addr_sig;
        m &in_coeff_base_addr_sig;
        m &out_data_base_addr_sig;
        m &nsamples_sig;
        m &nvectors_sig;
        m &niters_sig;
    }


    //Declare constructors

    conf_info()
    {
        this->cmd_sig = 0;
        this->in_data_base_addr_sig = 0;
        this->in_coeff_base_addr_sig = 0;
        this->out_data_base_addr_sig = 0;
        this->nsamples_sig = 0;
        this->nvectors_sig = 0;
        this->niters_sig = 0;
    }

    conf_info(
        uint32_t cmd_sig,
        uint32_t in_data_base_addr_sig,
        uint32_t in_coeff_base_addr_sig,
        uint32_t out_data_base_addr_sig,
        uint32_t nsamples_sig,
        uint32_t nvectors_sig,
        uint32_t niters_sig
        )
    {
        this->cmd_sig=cmd_sig;
        this->in_data_base_addr_sig =in_data_base_addr_sig;
        this->in_coeff_base_addr_sig =in_coeff_base_addr_sig;
        this->out_data_base_addr_sig =out_data_base_addr_sig;
        this->nsamples_sig=nsamples_sig;
        this->nvectors_sig=nvectors_sig;
        this->niters_sig=niters_sig;

    }

    // VCD dumping function

   inline friend void sc_trace(sc_trace_file *tf, const conf_info &v, const std::string &NAME)
    {
        sc_trace(tf,v.cmd_sig, NAME + ".cmd_sig");
        sc_trace(tf,v.in_data_base_addr_sig, NAME + ".in_data_base_addr_sig");
        sc_trace(tf,v.in_coeff_base_addr_sig, NAME + ".in_coeff_base_addr_sig");
        sc_trace(tf,v.out_data_base_addr_sig, NAME + ".out_data_base_addr_sig");
        sc_trace(tf,v.nsamples_sig, NAME + ".nsamples_sig");
        sc_trace(tf,v.nvectors_sig, NAME + ".nvectors_sig");
        sc_trace(tf,v.niters_sig, NAME + ".niters_sig");

    }

    // Stream to text for transaction logging

    friend ostream& operator << (ostream& os, conf_info const &conf_info)
    {
        os << "{";
        os << "cmd_sig = " << conf_info.cmd_sig << ", ";
        os << "in_data_base_addr_sig = " << conf_info.in_data_base_addr_sig << ", ";
        os << "in_coeff_base_addr_sig = " << conf_info.in_coeff_base_addr_sig << ", ";
        os << "out_data_base_addr_sig = " << conf_info.out_data_base_addr_sig << ", ";
        os << "nvectors_sig = " << conf_info.nvectors_sig << ", ";
        os << "niters_sig = " << conf_info.niters_sig << ", ";
        os << "}";
        return os;
    }

};

#endif // __MAC_CONF_INFO_HPP__
