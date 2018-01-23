`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 14.10.2017 12:32:01
// Design Name: 
// Module Name: bit_slip_Generator
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


module bitslip_Generator(
    input clk_40MHz,
    input rst_n,
    input bitslip_ena,
    input [7:0]data_in,
    output reg bitslip,
    output reg success,
    output reg busy 
    );
    //reg bitslip_out;
    reg bitslip_ena_p;
    
    
     
    
   
    //reg success;
    //reg busy;
    reg wait_cnt_en;
    reg wait_cnt_rst;
    reg Bitslip_cnt_en;
    reg Bitslip_cnt_rst;
    reg [3:0]wait_cnt;
    reg [4:0]Bitslip_cnt;
    reg [7:0]data_in_p;
    
       parameter IDLE    = 5'b00001;
       parameter CHECK   = 5'b00010;
       parameter BITSLIP = 5'b00100;
       parameter WAIT    = 5'B01000;
       parameter FINISH  = 5'b10000;
    
       reg [4:0] state = IDLE;
    
       always @(posedge clk_40MHz)
          if (~rst_n) begin
             state <= IDLE;
             bitslip<=0;
             busy<=0;
             success<=0;
             wait_cnt_en<=0;
             Bitslip_cnt_en<=0;
             wait_cnt_rst<=1;
             Bitslip_cnt_rst<=1;
          end
          else
          begin  //defaults
           
           busy<=1;
           //success<=0;
           bitslip<=0;
           wait_cnt_en<=0;
           wait_cnt_rst<=0;
           Bitslip_cnt_en<=0;
           Bitslip_cnt_rst<=0;
             case (state)
                   
            
             
                IDLE : begin
                   if (~bitslip_ena_p & bitslip_ena)
                      state <= CHECK;
                       
                  else
                        busy<=0;
                        wait_cnt_en<=0;
                        wait_cnt_rst<=1;
                        Bitslip_cnt_en<=0;
                        Bitslip_cnt_rst<=1;
                      end       
                CHECK : begin
                    
                   if ((data_in == 8'h7e && data_in_p == 8'h81) || ( data_in == 8'h81 && data_in_p == 8'h7e) )begin
                      state <= FINISH;
                      success<=1;end
                     else if (Bitslip_cnt<=10)
                      state <= BITSLIP;
                    else if(Bitslip_cnt>10)begin
                     success<=0;
                     state<=FINISH;end
                   else
                      state <= CHECK;
                      end
                BITSLIP : begin
                   bitslip<=1;
		   Bitslip_cnt_en<=1;	
                   state<=WAIT;
                   end
                WAIT : begin
                    wait_cnt_en<=1;
                   if (wait_cnt==3)begin
                      state <= CHECK;
		      wait_cnt_rst<=1;end
                   else
                      state <= WAIT;
                      end
                
                
                FINISH : begin
                       state <= IDLE;
                       busy<=0;
                       end
                       
             endcase
           end                     
           
           
           
           
           always@(posedge clk_40MHz)
           if(~rst_n)
           wait_cnt<=0;
           else if(wait_cnt_rst)
           wait_cnt<=0;
           else if(wait_cnt_en)
           wait_cnt<=wait_cnt+1;


        ////////////////////////////bislip counter, if no success in 10 shifts ,it means vfat3 is not responding    
           always@(posedge clk_40MHz)
                      if(~rst_n)
                      Bitslip_cnt<=0;
                      else if(Bitslip_cnt_rst)
                      Bitslip_cnt<=0;
                      else if(Bitslip_cnt_en) 
                      Bitslip_cnt<=Bitslip_cnt+1; 
            
          
          
      ////////////////////////////bislip start signal(from processor) bitslip_ena 
            always@(posedge clk_40MHz)
                         if(~rst_n)
                             bitslip_ena_p<=0;
                         else     
                         bitslip_ena_p<=bitslip_ena;
      ///////////////////////////////////////////////////////////////////////////////////////////////
          
                          
     
     
     ////////////////data_in one last value ////////////////////////////////////////////////////////      
             
             
             
             always@(posedge clk_40MHz)
                if(~rst_n)
                  data_in_p<=0;
                else     
                  data_in_p<=data_in;
           
           
endmodule