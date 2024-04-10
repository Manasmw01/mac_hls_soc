
// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __ACCSPECS__
#define __ACCSPECS__

#pragma once

#include "axi4_segment.h"
#include "nvhls_connections.h"
#include <nvhls_int.h>
#include <nvhls_types.h>
#include <nvhls_vector.h>
#include "mac_conf_info.hpp"
#include <ArbitratedScratchpadDP.h>

//Default Parameters for the
//computation Kernel

#define NSAMPLES 4
#define NVECTORS 4
#define NITERS 8
#define DMA_WIDTH 32

//Struct for indexing the configuration registers

struct dma_address_map {
    uint32_t cmd_sig;
    uint32_t in_data_base_addr_sig;
    uint32_t in_coeff_base_addr_sig;
    uint32_t out_data_base_addr_sig;
    uint32_t nsamples_sig;
    uint32_t nvectors_sig;
    uint32_t niters_sig;
};

//Default configutation for AXI4

namespace custom_axi {
    namespace cfg {
        struct standard {
            enum {
                dataWidth = 32,
                useVariableBeatSize = 0,
                useMisalignedAddresses = 0,
                useLast = 1,
                useWriteStrobes = 1,
                useBurst = 1, useFixedBurst = 0, useWrapBurst = 0, maxBurstSize = 256,
                useQoS = 0, useLock = 0, useProt = 0, useCache = 0, useRegion = 0,
                aUserWidth = 0, wUserWidth = 0, bUserWidth = 0, rUserWidth = 0,
                addrWidth = 32,
                idWidth = 4,
                useWriteResponses = 1,
            };
        };
    };
};

//Custom Datatypes for synthesis

typedef ac_int<32> FPDATA_WORD32;
typedef ac_int<16> FPDATA_WORD16;
typedef ac_int<64> FPDATA_WORD64;

typedef NVUINTW(32) DATA_TYPE32;
typedef NVUINTW(16) DATA_TYPE16;
typedef NVUINTW(64) DATA_TYPE64;

template<unsigned int kAddressSz>
struct Address{
    typedef NVUINTW(kAddressSz) value;
};

//Scratchpads parameters ad access datatypes

// # input write ports
const unsigned int inwp = 1;
const unsigned int incwp = 1;
const unsigned int outwp = 1;

// # input read ports
const unsigned int inrp = 1;
const unsigned int incrp = 1;
const unsigned int outrp = 1;

// # banks
const unsigned int inbks = 1;
const unsigned int incbks = 1;
const unsigned int outbks = 1;

// # entries per bank
const unsigned int inebks = NSAMPLES*NVECTORS;
const unsigned int incebks = NSAMPLES*NVECTORS;
const unsigned int outebks = NITERS;

// # address space
const unsigned int in_as = nvhls::index_width<inbks * inebks>::val;
const unsigned int inc_as = nvhls::index_width<incbks * incebks>::val;
const unsigned int out_as = nvhls::index_width<outbks * outebks>::val;


template<unsigned int kAddressSz, unsigned int numports, unsigned int wordwidth>
class plm_WR : public nvhls_message{
public:
    nvhls::nv_scvector<NVUINTW(kAddressSz), numports> indx;
    nvhls::nv_scvector<NVUINTW(wordwidth), numports> data;

    static const unsigned int width = nvhls::nv_scvector<NVUINTW(kAddressSz), numports>::width + nvhls::nv_scvector<NVUINTW(wordwidth), numports>::width;
    template <unsigned int Size>
    void Marshall(Marshaller<Size>& m) {
        m & indx;
        m & data;
    }
    plm_WR() {
        Reset();
    }
    void Reset() {
        indx = 0;
        data = 0;
    }
};

template <unsigned int kAddressSz, unsigned int numports>
class plm_RRq : public nvhls_message {
public:
    nvhls::nv_scvector<NVUINTW(kAddressSz), numports> indx;
    nvhls::nv_scvector<bool, numports> val;

    static const unsigned int width = nvhls::nv_scvector<NVUINTW(kAddressSz), numports>::width + nvhls::nv_scvector<bool, numports>::width;
    template <unsigned int Size>
    void Marshall(Marshaller<Size> &m) {
        m &indx;
        m &val;
    }

    plm_RRq() {
        Reset();
    }
    void Reset() {
        indx = 0;
        val = 0;
    }
};

template <unsigned int numports, unsigned int wordwidth>
class plm_RRs : public nvhls_message {
public:
    nvhls::nv_scvector<NVUINTW(wordwidth), numports> data;
    static const unsigned int width = nvhls::nv_scvector<NVUINTW(wordwidth), numports>::width;
    template <unsigned int Size>
    void Marshall(Marshaller<Size> &m) {
        m &data;
    }

    plm_RRs() {
        Reset();
    }

    void Reset() {
        data = 0;
    }
};

template<unsigned int regsnum, unsigned int regwidth>
class wregs : public nvhls_message{
public:
    nvhls::nv_scvector<NVUINTW(regwidth), regsnum> data;

    static const unsigned int width =nvhls::nv_scvector<NVUINTW(regwidth), regsnum>::width;
    template <unsigned int Size>
    void Marshall(Marshaller<Size>& m) {
        m & data;
    }
    wregs() {
        Reset();
    }
    void Reset() {
        data = 0;
    }
};


#endif
