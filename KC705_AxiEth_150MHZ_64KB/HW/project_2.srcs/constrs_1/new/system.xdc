#set_property LOC AD11 [ get_ports sys_clk_n]
#set_property LOC AD12 [ get_ports sys_clk_p]
set_property PACKAGE_PIN AB7 [get_ports reset]
set_property IOSTANDARD LVCMOS15 [get_ports reset]
# additional constraints
#
create_clock -period 5.000 -name sys_clk_pin [get_ports sys_clk_p]
# Added for RevC board

create_pblock pblock_i
add_cells_to_pblock [get_pblocks pblock_i] [get_cells -quiet [list mb_subsystem_i]]
resize_pblock [get_pblocks pblock_i] -add {CLOCKREGION_X0Y0:CLOCKREGION_X1Y3}

set_property PACKAGE_PIN AC29 [get_ports RXD_P]
set_property IOSTANDARD LVDS_25 [get_ports RXD_P]
set_property IOSTANDARD LVDS_25 [get_ports RXD_N]
set_property IBUF_LOW_PWR FALSE [get_ports sys_clk_p]
set_property IOSTANDARD LVDS [get_ports sys_clk_p]
set_property IOSTANDARD LVDS [get_ports sys_clk_n]


set_property PACKAGE_PIN AD29 [get_ports rxclk_320_p]
set_property PACKAGE_PIN AB29 [get_ports txd_p]

set_property PACKAGE_PIN AH20 [get_ports iic_main_scl_io]
set_property PACKAGE_PIN AG20 [get_ports iic_main_sda_io]
set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]
set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets clk]
