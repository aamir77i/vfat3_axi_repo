`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 12.04.2017 20:57:39
// Design Name: 
// Module Name: AP_Generator
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


module AP_Generator(
    input clk,
    input rst_n,
    input datavalid,
    input [7:0]data_in,
    output [7:0] data_out
    );
    reg [7:0] data_out;
    reg [7:0] AP;
    
    always@(posedge clk)
    if(~rst_n)
    data_out<=0;
    else if(datavalid)
    data_out<=data_in;
    else 
    data_out<=AP;
     
    
    
    
    always@(posedge clk)
    if(~rst_n)
    AP<=0;
    else 
    AP<=~AP;
    
        
endmodule
