# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "F1" -parent ${Page_0}
  ipgui::add_param $IPINST -name "F2" -parent ${Page_0}
  ipgui::add_param $IPINST -name "FILTER_DATA" -parent ${Page_0}
  ipgui::add_param $IPINST -name "H0A" -parent ${Page_0}
  ipgui::add_param $IPINST -name "H0B" -parent ${Page_0}
  ipgui::add_param $IPINST -name "H1A" -parent ${Page_0}
  ipgui::add_param $IPINST -name "H1B" -parent ${Page_0}
  ipgui::add_param $IPINST -name "IDLE" -parent ${Page_0}
  ipgui::add_param $IPINST -name "NUM_BYTES" -parent ${Page_0}
  ipgui::add_param $IPINST -name "SEND_PACKET" -parent ${Page_0}


}

proc update_PARAM_VALUE.F1 { PARAM_VALUE.F1 } {
	# Procedure called to update F1 when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.F1 { PARAM_VALUE.F1 } {
	# Procedure called to validate F1
	return true
}

proc update_PARAM_VALUE.F2 { PARAM_VALUE.F2 } {
	# Procedure called to update F2 when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.F2 { PARAM_VALUE.F2 } {
	# Procedure called to validate F2
	return true
}

proc update_PARAM_VALUE.FILTER_DATA { PARAM_VALUE.FILTER_DATA } {
	# Procedure called to update FILTER_DATA when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.FILTER_DATA { PARAM_VALUE.FILTER_DATA } {
	# Procedure called to validate FILTER_DATA
	return true
}

proc update_PARAM_VALUE.H0A { PARAM_VALUE.H0A } {
	# Procedure called to update H0A when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.H0A { PARAM_VALUE.H0A } {
	# Procedure called to validate H0A
	return true
}

proc update_PARAM_VALUE.H0B { PARAM_VALUE.H0B } {
	# Procedure called to update H0B when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.H0B { PARAM_VALUE.H0B } {
	# Procedure called to validate H0B
	return true
}

proc update_PARAM_VALUE.H1A { PARAM_VALUE.H1A } {
	# Procedure called to update H1A when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.H1A { PARAM_VALUE.H1A } {
	# Procedure called to validate H1A
	return true
}

proc update_PARAM_VALUE.H1B { PARAM_VALUE.H1B } {
	# Procedure called to update H1B when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.H1B { PARAM_VALUE.H1B } {
	# Procedure called to validate H1B
	return true
}

proc update_PARAM_VALUE.IDLE { PARAM_VALUE.IDLE } {
	# Procedure called to update IDLE when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.IDLE { PARAM_VALUE.IDLE } {
	# Procedure called to validate IDLE
	return true
}

proc update_PARAM_VALUE.NUM_BYTES { PARAM_VALUE.NUM_BYTES } {
	# Procedure called to update NUM_BYTES when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.NUM_BYTES { PARAM_VALUE.NUM_BYTES } {
	# Procedure called to validate NUM_BYTES
	return true
}

proc update_PARAM_VALUE.SEND_PACKET { PARAM_VALUE.SEND_PACKET } {
	# Procedure called to update SEND_PACKET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.SEND_PACKET { PARAM_VALUE.SEND_PACKET } {
	# Procedure called to validate SEND_PACKET
	return true
}


proc update_MODELPARAM_VALUE.IDLE { MODELPARAM_VALUE.IDLE PARAM_VALUE.IDLE } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.IDLE}] ${MODELPARAM_VALUE.IDLE}
}

proc update_MODELPARAM_VALUE.FILTER_DATA { MODELPARAM_VALUE.FILTER_DATA PARAM_VALUE.FILTER_DATA } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.FILTER_DATA}] ${MODELPARAM_VALUE.FILTER_DATA}
}

proc update_MODELPARAM_VALUE.SEND_PACKET { MODELPARAM_VALUE.SEND_PACKET PARAM_VALUE.SEND_PACKET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.SEND_PACKET}] ${MODELPARAM_VALUE.SEND_PACKET}
}

proc update_MODELPARAM_VALUE.F1 { MODELPARAM_VALUE.F1 PARAM_VALUE.F1 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.F1}] ${MODELPARAM_VALUE.F1}
}

proc update_MODELPARAM_VALUE.F2 { MODELPARAM_VALUE.F2 PARAM_VALUE.F2 } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.F2}] ${MODELPARAM_VALUE.F2}
}

proc update_MODELPARAM_VALUE.H0A { MODELPARAM_VALUE.H0A PARAM_VALUE.H0A } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.H0A}] ${MODELPARAM_VALUE.H0A}
}

proc update_MODELPARAM_VALUE.H0B { MODELPARAM_VALUE.H0B PARAM_VALUE.H0B } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.H0B}] ${MODELPARAM_VALUE.H0B}
}

proc update_MODELPARAM_VALUE.H1A { MODELPARAM_VALUE.H1A PARAM_VALUE.H1A } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.H1A}] ${MODELPARAM_VALUE.H1A}
}

proc update_MODELPARAM_VALUE.H1B { MODELPARAM_VALUE.H1B PARAM_VALUE.H1B } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.H1B}] ${MODELPARAM_VALUE.H1B}
}

proc update_MODELPARAM_VALUE.NUM_BYTES { MODELPARAM_VALUE.NUM_BYTES PARAM_VALUE.NUM_BYTES } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.NUM_BYTES}] ${MODELPARAM_VALUE.NUM_BYTES}
}

