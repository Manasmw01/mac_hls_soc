/* Copyright 2022 Columbia University, SLD Group */

#include "system.hpp"
#include <systemc.h>
#include <mc_scverify.h>

sc_trace_file *trace_file_ptr;

int sc_main(int argc, char *argv[])
{
    sc_report_handler::set_actions(SC_WARNING, SC_DO_NOTHING);

    sc_trace_file *trace_file_ptr = sc_trace_static::setup_trace_file("trace");

    system_t system_inst("system_inst");
    trace_hierarchy(&system_inst, trace_file_ptr);

    sc_start();

    return 0;
}
