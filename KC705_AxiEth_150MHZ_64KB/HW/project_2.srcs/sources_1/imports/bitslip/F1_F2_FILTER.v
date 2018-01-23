`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 25.04.2017 09:46:28
// Design Name: 
// Module Name: rx_fifo_int
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


module F1_F2_FILTER(
    input clk40,
    input    rst_n,
	input bit_aligned,
       input  [7:0] d_in,

    output  reg dv,
     output  [31:0] d_out//[31:0]
        //reg [2:0]state_p,
         // data_valid, for FIFO dv is wr_en
    );
                    
    // Register for input data (to cut the combinatorial path from deserializer
    wire [7:0] d_in_n;
    reg  [7:0] d_in_r;
    
    reg [2:0] state;
    
    // BCd counter
    //wire [23:0] BCd_n;
    reg  [23:0] BCd;
    
    // Counter counting number of sent bytes (up to 26)
    reg [5:0] cnt;
    
    // States
    parameter [2:0]     IDLE 		= 3'b001,
			FILTER_DATA 	= 3'b010,
                        SEND_PACKET 	= 3'b100;
    
    // Fillers and headers          
    parameter [7:0] F1 	= 8'h7e,
                    F2 	= 8'h81,
                    H0A = 8'h1E,
                    H0B = 8'h5E,
                    H1A = 8'h1A,
                    H1B = 8'h56;
                    
    parameter [4:0] NUM_BYTES = 5'd25; // it should be around 25
    
    
    //assign d_in_n = d_in;
    
    // Ouput data is concatenation of BCd and 8-bit input data
    assign d_out = {BCd, d_in_r};
    
    // Always count
    //assign BCd_n = BCd_p+1;
    
    always@(posedge clk40) 
	if(~rst_n)
		begin
        	// default assignments
        	state <= IDLE;
        	cnt <= 0;
        	dv <= 0;
        	end
	else 
	begin
	dv<=0;
        case(state)
		   IDLE:begin
			dv<=0;
			if(bit_aligned)
			state<= FILTER_DATA;
			end
 	           FILTER_DATA :
                // if data is different than F1 and F2 or if BCd counter is 0
                if( ((d_in!= F1) && (d_in != F2)) ) begin
                    dv <= 1;
                    if ((d_in == H0A) || (d_in == H0B) || (d_in == H1A)  || (d_in == H1B)) begin
                       // pass data to 
                        state <= SEND_PACKET;
                        cnt <= 0;
                    end
                end
                    
            SEND_PACKET : begin
                dv <= 1;
                if(cnt >= NUM_BYTES) begin
                    state <= FILTER_DATA;    
                end 
                else begin
                    cnt <= cnt+1;  
                end
            end
        endcase
    end
    
    always@(posedge clk40 ) begin
        if(~rst_n) begin
            //d_in_p <= 0;
            //state_p <= IDLE;
            BCd <= 0;//BCd_n;
	d_in_r<=0;
            //cnt_p <= 0;
	
        end
        else begin
           // d_in_p <= d_in_n;
           // state_p <= state_n;
            BCd <= BCd+1;
	d_in_r<=d_in;	
            //cnt_p <= cnt_n;
	
        end
    end              





    
endmodule
