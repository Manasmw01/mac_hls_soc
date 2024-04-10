
###  mac_hls1 ###

The synthesizable MAC is described in SystemC using seven SC_THREADs:

Two threads for configuration and interrupt:
- `config`
- `done_read`

Three standards threads of the accelerator structure:
- `load_input`
- `compute_kernel`
- `store_output`

An additional thread to send read requests to the input data and coefficients scratchpads for the computatiion:
- `compute_kernel_req`

An additional thread to send read requests to the output accumulation scratchpads for storing data back to main memory:
- `store_output_req`

The local memory of the accelerator is organized as follows:
Two scratchpads are used to implement a ping-pong buffer for the MAC input coefficients.
Two scratchpads are used to implement a ping-pong buffer for the MAC input data.
Two scratchpads are used to implement a ping-pong  accumulation for the MAC results.

Testbench modeling and DUT-testbench communcation replicates the `mac_axi` design. Threads synchronization is based on `Connections::SyncChannel` as in the other examples. However, more channels are required as a consequence of the higher number of threads in the design.

With respect to `mac_axi`, the design includes the additional synthesizable logic in the `#ifdef HLS_READY` branches and the old logic in the `#else` branches.

Refer to the following targets to launch from the `/syn` subfolder:

```
# Run behavioral SystemC simulation of non-synthesizable version
$ make run

# Run behavioral SystemC simulation of synthesizable version
$ make run_syn

# Synthesize the synthesizable version using Catapult-HLS (shell option)
$ make hls

# Synthesize the synthesizable version using Catapult-HLS GUI
$ make hls-gui

#Simulate the synthesized design using Modelsim (shell-option)
$ make sim

#Simulate the synthesized design using Modelsim GUI
$ make sim-gui

```
