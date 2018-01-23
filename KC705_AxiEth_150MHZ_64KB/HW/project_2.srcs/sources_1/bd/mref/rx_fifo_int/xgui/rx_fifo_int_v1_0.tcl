# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "BCd_w" -parent ${Page_0}
  ipgui::add_param $IPINST -name "cmd_w" -parent ${Page_0}
  ipgui::add_param $IPINST -name "fifo_w" -parent ${Page_0}


}

proc update_PARAM_VALUE.BCd_w { PARAM_VALUE.BCd_w } {
	# Procedure called to update BCd_w when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.BCd_w { PARAM_VALUE.BCd_w } {
	# Procedure called to validate BCd_w
	return true
}

proc update_PARAM_VALUE.cmd_w { PARAM_VALUE.cmd_w } {
	# Procedure called to update cmd_w when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.cmd_w { PARAM_VALUE.cmd_w } {
	# Procedure called to validate cmd_w
	return true
}

proc update_PARAM_VALUE.fifo_w { PARAM_VALUE.fifo_w } {
	# Procedure called to update fifo_w when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.fifo_w { PARAM_VALUE.fifo_w } {
	# Procedure called to validate fifo_w
	return true
}


proc update_MODELPARAM_VALUE.cmd_w { MODELPARAM_VALUE.cmd_w PARAM_VALUE.cmd_w } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.cmd_w}] ${MODELPARAM_VALUE.cmd_w}
}

proc update_MODELPARAM_VALUE.fifo_w { MODELPARAM_VALUE.fifo_w PARAM_VALUE.fifo_w } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.fifo_w}] ${MODELPARAM_VALUE.fifo_w}
}

proc update_MODELPARAM_VALUE.BCd_w { MODELPARAM_VALUE.BCd_w PARAM_VALUE.BCd_w } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.BCd_w}] ${MODELPARAM_VALUE.BCd_w}
}

