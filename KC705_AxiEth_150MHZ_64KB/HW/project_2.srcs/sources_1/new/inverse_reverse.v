`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 17.10.2017 10:17:38
// Design Name: 
// Module Name: inverse_reverse
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


module inverse_reverse(
    input [7:0] data_in,
    output reg [7:0] data_out,
    input clk,
    input rst_n,
    input reverse,
    input invert,
    output reg valid
    );
    
      
        always@(posedge clk )
        if(~rst_n)
        begin
        data_out<=0;
        valid<=0;
        end
        else if(invert & reverse )//inverse & reverse both 
            begin
                data_out[0]    <=  ~data_in[7];
                data_out[1]    <=  ~data_in[6];
                data_out[2]    <=  ~data_in[5];
                data_out[3]    <=  ~data_in[4];
                data_out[4]    <=  ~data_in[3];
                data_out[5]    <=  ~data_in[2];
                data_out[6]    <=  ~data_in[1];
                data_out[7]    <=  ~data_in[0];
                
                valid       <=  1;
            end
            
         else if(~invert & reverse )//reverse only
                        begin
                            data_out[0]    <=  data_in[7];
                            data_out[1]    <=  data_in[6];
                            data_out[2]    <=  data_in[5];
                            data_out[3]    <=  data_in[4];
                            data_out[4]    <=  data_in[3];
                            data_out[5]    <=  data_in[2];
                            data_out[6]    <=  data_in[1];
                            data_out[7]    <=  data_in[0];
                            valid       <=  1;
                        end
                        
        else if(invert & ~reverse )//invert only
                                                begin
                           data_out[0]    <=  ~data_in[0];
                           data_out[1]    <=  ~data_in[1];
                           data_out[2]    <=  ~data_in[2];
                           data_out[3]    <=  ~data_in[3];
                           data_out[4]    <=  ~data_in[4];
                           data_out[5]    <=  ~data_in[5];
                           data_out[6]    <=  ~data_in[6];
                           data_out[7]    <=  ~data_in[7];
                           valid       <=  1;
                                                end 
                                                
       else if(~invert & ~reverse )//none
                          begin
                          data_out[0]    <=  data_in[0];
                          data_out[1]    <=  data_in[1];
                          data_out[2]    <=  data_in[2];
                          data_out[3]    <=  data_in[3];
                          data_out[4]    <=  data_in[4];
                          data_out[5]    <=  data_in[5];
                          data_out[6]    <=  data_in[6];
                          data_out[7]    <=  data_in[7];
                          valid       <=  1;
                          end
            else 
                begin
                    valid<=1;
                    data_out<=data_out;
                end
    endmodule


