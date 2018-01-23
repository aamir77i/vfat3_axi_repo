# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "BITSLIP" -parent ${Page_0}
  ipgui::add_param $IPINST -name "CHECK" -parent ${Page_0}
  ipgui::add_param $IPINST -name "FINISH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "IDLE" -parent ${Page_0}
  ipgui::add_param $IPINST -name "WAIT" -parent ${Page_0}


}

proc update_PARAM_VALUE.BITSLIP { PARAM_VALUE.BITSLIP } {
	# Procedure called to update BITSLIP when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.BITSLIP { PARAM_VALUE.BITSLIP } {
	# Procedure called to validate BITSLIP
	return true
}

proc update_PARAM_VALUE.CHECK { PARAM_VALUE.CHECK } {
	# Procedure called to update CHECK when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.CHECK { PARAM_VALUE.CHECK } {
	# Procedure called to validate CHECK
	return true
}

proc update_PARAM_VALUE.FINISH { PARAM_VALUE.FINISH } {
	# Procedure called to update FINISH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.FINISH { PARAM_VALUE.FINISH } {
	# Procedure called to validate FINISH
	return true
}

proc update_PARAM_VALUE.IDLE { PARAM_VALUE.IDLE } {
	# Procedure called to update IDLE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.IDLE { PARAM_VALUE.IDLE } {
	# Procedure called to validate IDLE
	return true
}

proc update_PARAM_VALUE.WAIT { PARAM_VALUE.WAIT } {
	# Procedure called to update WAIT when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.WAIT { PARAM_VALUE.WAIT } {
	# Procedure called to validate WAIT
	return true
}


proc update_MODELPARAM_VALUE.IDLE { MODELPARAM_VALUE.IDLE PARAM_VALUE.IDLE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.IDLE}] ${MODELPARAM_VALUE.IDLE}
}

proc update_MODELPARAM_VALUE.CHECK { MODELPARAM_VALUE.CHECK PARAM_VALUE.CHECK } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.CHECK}] ${MODELPARAM_VALUE.CHECK}
}

proc update_MODELPARAM_VALUE.BITSLIP { MODELPARAM_VALUE.BITSLIP PARAM_VALUE.BITSLIP } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.BITSLIP}] ${MODELPARAM_VALUE.BITSLIP}
}

proc update_MODELPARAM_VALUE.WAIT { MODELPARAM_VALUE.WAIT PARAM_VALUE.WAIT } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.WAIT}] ${MODELPARAM_VALUE.WAIT}
}

proc update_MODELPARAM_VALUE.FINISH { MODELPARAM_VALUE.FINISH PARAM_VALUE.FINISH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.FINISH}] ${MODELPARAM_VALUE.FINISH}
}

