/* Copyright 2021 Columbia University SLD Group */

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdio.h>

#include "system.hpp"

std::ofstream ofs;

#include "mac_specs.hpp"

void system_t::load_regs(void)
{

    assert(driver->do_write(offsetof(dma_address_map,in_data_base_addr_sig),    in_data_addr << 2));
    assert(driver->do_write(offsetof(dma_address_map,in_coeff_base_addr_sig),  in_coeff_addr << 2));
    assert(driver->do_write(offsetof(dma_address_map,out_data_base_addr_sig), out_data_addr << 2));
    assert(driver->do_write(offsetof(dma_address_map,nsamples_sig), NSAMPLES));
    assert(driver->do_write(offsetof(dma_address_map,nvectors_sig), NVECTORS));
    assert(driver->do_write(offsetof(dma_address_map,niters_sig), NITERS));

}

void system_t::load_memory(void)
{


// Allocate memory
    in_data_size = NSAMPLES * NVECTORS * NITERS;
    in_coeff_size = NSAMPLES * NVECTORS * NITERS / 2;
    out_data_size = NVECTORS * NITERS * 2;

    in_data_addr = 0;
    in_coeff_addr = in_data_size;
    out_data_addr = in_data_size + in_coeff_size;


    mem_size = in_data_size + in_coeff_size + out_data_size;

    // -- Generate random input and store it to memory
    for (uint32_t i = 0; i < in_data_size; i++) {
        uint32_t mem_addr = in_data_addr + i;
        int32_t tmpData =  rand()%10;
        memory.data[mem_addr] = tmpData;
    }
    for (uint32_t i = 0; i < in_coeff_size; i++) {
        uint32_t mem_addr = in_coeff_addr + i;
        int32_t tmpData1 = rand() % 10;
        int32_t tmpData2 = rand() % 10;
        memory.data[mem_addr].range(15,0) = tmpData1;
        memory.data[mem_addr].range(31,16) = tmpData2;
    }

    // -- Compute golden output
    for (uint32_t i = 0; i < NITERS; i++) {
        for (uint32_t k = 0; k < NVECTORS; k++) {
            int64_t tmpAcc = 0;
            for (uint32_t j = 0; j < NSAMPLES; j++) {
                int32_t tmpData = memory.data[in_data_addr + i * NSAMPLES * NVECTORS + k * NSAMPLES+ j].to_int();
                int32_t tmpCoeff;
                if (j % 2)
                    tmpCoeff = memory.data[in_coeff_addr + (i * NSAMPLES * NVECTORS + k * NSAMPLES + j) / 2].range(31, 16).to_int();
                else
                    tmpCoeff = memory.data[in_coeff_addr + (i * NSAMPLES * NVECTORS + k * NSAMPLES+ j) / 2].range(15, 0).to_int();
                tmpAcc += (int64_t) tmpData * (int64_t) tmpCoeff;
            }
            golden_output[i * NVECTORS + k] = tmpAcc;
        }
    }

    CCS_LOG("Load memory completed");

    wait();

}

void system_t::dump_memory(void)
{
    uint32_t word1;
    int32_t word2;

    for (uint32_t i = 0; i < NITERS*NVECTORS; ++i)
    {
        word1 = memory.data[out_data_addr + i * 2].to_int();
        word2 = memory.data[out_data_addr + i * 2 + 1].to_int();
        output[i] = (((int64_t) word2) << 32) | ((uint64_t) word1);
    }

    CCS_LOG("Dump memory completed");
}

#define ERROR_THRESHOLD 0.01

void system_t::validate(void)
{
    uint32_t errors = 0;

    for (uint32_t i = 0; i < NITERS*NVECTORS; i++)
    {
        if (golden_output[i] != output[i]) {
            CCS_LOG("Error: gold["<<i<<"] "<< golden_output[i] <<" != out["<<i<<"] "<< output[i]);
            errors++;
        }
    }

    CCS_LOG("Mac complete with " << errors << " errors!");
}

