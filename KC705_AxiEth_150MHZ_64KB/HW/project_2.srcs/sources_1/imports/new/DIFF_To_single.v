`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 14.04.2017 12:01:47
// Design Name: 
// Module Name: DIFF_To_single
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module DIFF_To_single(
    input RXD_P,
    input RXD_N,
    output RXD
    );
    
    wire RXD;
    
    IBUFDS #(
          .DIFF_TERM("TRUE"),       // Differential Termination
          .IBUF_LOW_PWR("FALSE"),     // Low power="TRUE", Highest performance="FALSE" 
          .IOSTANDARD("DEFAULT")     // Specify the input I/O standard
       ) IBUFDS_inst (
          .O(RXD),  // Buffer output
          .I(RXD_P),  // Diff_p buffer input (connect directly to top-level port)
          .IB(RXD_N) // Diff_n buffer input (connect directly to top-level port)
       );
endmodule
