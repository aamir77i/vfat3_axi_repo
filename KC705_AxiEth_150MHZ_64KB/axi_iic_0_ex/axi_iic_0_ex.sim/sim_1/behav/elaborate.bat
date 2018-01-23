@echo off
set xv_path=C:\\Xilinx\\Vivado\\2017.2\\bin
call %xv_path%/xelab  -wto 610414d980b8451da40b1a1cf898d483 -m64 --debug typical --relax --mt 2 -L lib_pkg_v1_0_2 -L lib_cdc_v1_0_2 -L axi_lite_ipif_v3_0_4 -L interrupt_control_v3_1_4 -L axi_iic_v2_0_16 -L xil_defaultlib -L dist_mem_gen_v8_0_11 -L blk_mem_gen_v8_3_6 -L lib_bmg_v1_0_8 -L axi_traffic_gen_v2_0_14 -L unisims_ver -L unimacro_ver -L secureip -L xpm --snapshot axi_iic_0_exdes_tb_behav xil_defaultlib.axi_iic_0_exdes_tb xil_defaultlib.glbl -log elaborate.log
if "%errorlevel%"=="0" goto SUCCESS
if "%errorlevel%"=="1" goto END
:END
exit 1
:SUCCESS
exit 0
