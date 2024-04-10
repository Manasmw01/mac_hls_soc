#Copyright (c) 2011-2022 Columbia University, System Level Design Group
#SPDX-License-Identifier: Apache-2.0

project load mac.ccs

flow run /SCVerify/launch_make ./scverify/Verify_concat_sim_rtl_v_msim.mk {} SIMTOOL=msim sim
