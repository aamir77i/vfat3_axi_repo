`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 13.04.2017 10:44:54
// Design Name: 
// Module Name: OBUFDS_WRAPPER
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


module Single_to_diff(
    input in,
    output out_p,
    output out_n
    );
    
    wire out_p;
    wire out_n;
     OBUFDS #(
                .IOSTANDARD("LVDS_25"), // Specify the output I/O standard
                .SLEW("FAST")           // Specify the output slew rate
             ) Single_to_differential (
                .O(out_p),     // Diff_p output (connect directly to top-level port)
                .OB(out_n),   // Diff_n output (connect directly to top-level port)
                .I(in)      // Buffer input 
             );
endmodule
