//  (c) Copyright 2011-2013 Xilinx, Inc. All rights reserved.
//
//  This file contains confidential and proprietary information
//  of Xilinx, Inc. and is protected under U.S. and
//  international copyright and other intellectual property
//  laws.
//
//  DISCLAIMER
//  This disclaimer is not a license and does not grant any
//  rights to the materials distributed herewith. Except as
//  otherwise provided in a valid license issued to you by
//  Xilinx, and to the maximum extent permitted by applicable
//  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
//  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
//  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
//  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
//  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
//  (2) Xilinx shall not be liable (whether in contract or tort,
//  including negligence, or under any other theory of
//  liability) for any loss or damage of any kind or nature
//  related to, arising under or in connection with these
//  materials, including for any direct, or any indirect,
//  special, incidental, or consequential loss or damage
//  (including loss of data, profits, goodwill, or any type of
//  loss or damage suffered as a result of any action brought
//  by a third party) even if such damage or loss was
//  reasonably foreseeable or Xilinx had been advised of the
//  possibility of the same.
//
//  CRITICAL APPLICATIONS
//  Xilinx products are not designed or intended to be fail-
//  safe, or for use in any application requiring fail-safe
//  performance, such as life-support or safety devices or
//  systems, Class III medical devices, nuclear facilities,
//  applications related to the deployment of airbags, or any
//  other applications that could lead to death, personal
//  injury, or severe property or environmental damage
//  (individually and collectively, "Critical
//  Applications"). Customer assumes the sole risk and
//  liability of any use of Xilinx products in Critical
//  Applications, subject only to applicable laws and
//  regulations governing limitations on product liability.
//
//  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
//  PART OF THIS FILE AT ALL TIMES. 
//-----------------------------------------------------------------------------
//
// Register Slice
//   Generic single-channel AXI pipeline register on forward and/or reverse signal path
//
// Verilog-standard:  Verilog 2001
//--------------------------------------------------------------------------
//
// Structure:
//   axic_sync_clock_converter
//
//--------------------------------------------------------------------------

`timescale 1ps/1ps
`default_nettype none

(* DowngradeIPIdentifiedWarnings="yes" *)
module axis_clock_converter_v1_1_14_axisc_sync_clock_converter # (
///////////////////////////////////////////////////////////////////////////////
// Parameter Definitions
///////////////////////////////////////////////////////////////////////////////
  parameter C_FAMILY     = "virtex6",
  parameter integer C_PAYLOAD_WIDTH = 32,
  parameter integer C_S_ACLK_RATIO = 1,
  parameter integer C_M_ACLK_RATIO = 1,
  parameter integer C_MODE = 1  // 0 = light-weight (1-deep); 1 = fully-pipelined (2-deep)
  )
 (
///////////////////////////////////////////////////////////////////////////////
// Port Declarations
///////////////////////////////////////////////////////////////////////////////
  input wire                         SAMPLE_CYCLE_EARLY,
  input wire                         SAMPLE_CYCLE,
  // Slave side
  input  wire                        S_ACLK,
  input  wire                        S_ARESETN,
  input  wire [C_PAYLOAD_WIDTH-1:0]  S_PAYLOAD,
  input  wire                        S_VALID,
  output wire                        S_READY,

  // Master side
  input  wire                        M_ACLK,
  input  wire                        M_ARESETN,
  output wire [C_PAYLOAD_WIDTH-1:0]  M_PAYLOAD,
  output wire                        M_VALID,
  input  wire                        M_READY
);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Local parameters
////////////////////////////////////////////////////////////////////////////////
localparam [1:0] ZERO = 2'b10;
localparam [1:0] ONE  = 2'b11;
localparam [1:0] FULL = 2'b01;
localparam [1:0] INIT = 2'b00;
localparam integer P_LIGHT_WT = 0;
localparam integer P_FULLY_REG = 1;

////////////////////////////////////////////////////////////////////////////////
// Wires/Reg declarations
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// BEGIN RTL
////////////////////////////////////////////////////////////////////////////////

generate
  if (C_S_ACLK_RATIO == C_M_ACLK_RATIO) begin : gen_passthru
    assign M_PAYLOAD = S_PAYLOAD;
    assign M_VALID   = S_VALID;
    assign S_READY   = M_READY;      
  end else begin : gen_sync_clock_converter
    wire s_sample_cycle;
    wire s_sample_cycle_early;
    wire m_sample_cycle;
    wire m_sample_cycle_early;

    wire slow_aclk;
    wire slow_areset;
    reg  s_areset_r;
    reg  m_areset_r;
   
    reg  s_ready_r; 
    wire s_ready_ns; 
    reg  m_valid_r; 
    wire m_valid_ns; 
    reg  [C_PAYLOAD_WIDTH-1:0] m_payload_r;
    reg  [C_PAYLOAD_WIDTH-1:0] m_storage_r;
    wire [C_PAYLOAD_WIDTH-1:0] m_payload_ns; 
    wire [C_PAYLOAD_WIDTH-1:0] m_storage_ns; 
    reg  m_ready_hold;
    wire m_ready_sample;
    wire load_payload;
    wire load_storage;
    wire load_payload_from_storage;
    reg [1:0] state;
    reg [1:0] next_state;
    
    always @(posedge S_ACLK) begin
      s_areset_r <= ~(S_ARESETN & M_ARESETN);
    end
  
    always @(posedge M_ACLK) begin
      m_areset_r <= ~(S_ARESETN & M_ARESETN);
    end

    assign slow_aclk   = (C_S_ACLK_RATIO > C_M_ACLK_RATIO) ? M_ACLK   : S_ACLK;
    assign slow_areset = (C_S_ACLK_RATIO > C_M_ACLK_RATIO) ? m_areset_r : s_areset_r;
    assign s_sample_cycle_early = (C_S_ACLK_RATIO > C_M_ACLK_RATIO) ? SAMPLE_CYCLE_EARLY : 1'b1;
    assign s_sample_cycle       = (C_S_ACLK_RATIO > C_M_ACLK_RATIO) ? SAMPLE_CYCLE : 1'b1;
    assign m_sample_cycle_early = (C_S_ACLK_RATIO > C_M_ACLK_RATIO) ? 1'b1 : SAMPLE_CYCLE_EARLY;
    assign m_sample_cycle       = (C_S_ACLK_RATIO > C_M_ACLK_RATIO) ? 1'b1 : SAMPLE_CYCLE;

    // Output flop for S_READY, value is encoded into state machine.
    assign s_ready_ns = (C_S_ACLK_RATIO > C_M_ACLK_RATIO) ? state[1] & (state != INIT) : next_state[1];

    always @(posedge S_ACLK) begin 
      if (s_areset_r) begin
        s_ready_r <= 1'b0;
      end
      else begin
        s_ready_r <= s_sample_cycle_early ? s_ready_ns : 1'b0;
      end
    end

    assign S_READY = s_ready_r;

    // Output flop for M_VALID
    assign m_valid_ns = next_state[0];

    always @(posedge M_ACLK) begin 
      if (m_areset_r) begin
        m_valid_r <= 1'b0;
      end
      else begin
        m_valid_r <= m_sample_cycle ? m_valid_ns : m_valid_r & ~M_READY;
      end
    end

    assign M_VALID = m_valid_r;

    // Hold register for M_READY when M_ACLK is fast.
    always @(posedge M_ACLK) begin 
      if (m_areset_r) begin
        m_ready_hold <= 1'b0;
      end
      else begin
        m_ready_hold <= m_sample_cycle ? 1'b0 : m_ready_sample;
      end
    end

    assign m_ready_sample = (M_READY ) | m_ready_hold;
    // Output/storage flops for PAYLOAD
    assign m_payload_ns = ~load_payload ? m_payload_r :
                           load_payload_from_storage ? m_storage_r : 
                           S_PAYLOAD;

    assign m_storage_ns = (C_MODE == P_FULLY_REG) ? (load_storage ? S_PAYLOAD : m_storage_r) : {C_PAYLOAD_WIDTH{1'b0}};

    always @(posedge slow_aclk) begin 
      m_payload_r <= m_payload_ns;
      m_storage_r <= (C_MODE == P_FULLY_REG) ? m_storage_ns : 0;
    end

    assign M_PAYLOAD = m_payload_r;

    // load logic
    assign load_storage = (C_MODE == P_FULLY_REG) && (state != FULL);
    assign load_payload = m_ready_sample || (state == ZERO);
    assign load_payload_from_storage = (C_MODE == P_FULLY_REG) && (state == FULL) && m_ready_sample;
    
    // State machine
    always @(posedge slow_aclk) begin 
      state <= next_state;
    end

    always @* begin 
      if (slow_areset) begin 
        next_state = INIT;
      end else begin
        case (state)
          INIT: begin
            next_state = ZERO;
          end
          // No transaction stored locally
          ZERO: begin
            if (S_VALID) begin
              next_state = (C_MODE == P_FULLY_REG) ? ONE : FULL; // Push from empty
            end
            else begin
              next_state = ZERO;
            end
          end

          // One transaction stored locally
          ONE: begin
            if (m_ready_sample & ~S_VALID) begin 
              next_state = ZERO; // Read out one so move to ZERO
            end
            else if (~m_ready_sample & S_VALID) begin
              next_state = FULL;  // Got another one so move to FULL
            end
            else begin
              next_state = ONE;
            end
          end

          // Storage registers full
          FULL: begin 
            if (m_ready_sample) begin
              next_state = (C_MODE == P_FULLY_REG) ? ONE : ZERO; // Pop from full
            end
            else begin
              next_state = FULL;
            end
          end
          default: begin
            next_state = ZERO;
          end
        endcase // case (state)
      end
    end
  end // gen_sync_clock_converter
  endgenerate
endmodule // axisc_sync_clock_converter

`default_nettype wire


//  (c) Copyright 2011-2013 Xilinx, Inc. All rights reserved.
//
//  This file contains confidential and proprietary information
//  of Xilinx, Inc. and is protected under U.S. and
//  international copyright and other intellectual property
//  laws.
//
//  DISCLAIMER
//  This disclaimer is not a license and does not grant any
//  rights to the materials distributed herewith. Except as
//  otherwise provided in a valid license issued to you by
//  Xilinx, and to the maximum extent permitted by applicable
//  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
//  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
//  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
//  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
//  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
//  (2) Xilinx shall not be liable (whether in contract or tort,
//  including negligence, or under any other theory of
//  liability) for any loss or damage of any kind or nature
//  related to, arising under or in connection with these
//  materials, including for any direct, or any indirect,
//  special, incidental, or consequential loss or damage
//  (including loss of data, profits, goodwill, or any type of
//  loss or damage suffered as a result of any action brought
//  by a third party) even if such damage or loss was
//  reasonably foreseeable or Xilinx had been advised of the
//  possibility of the same.
//
//  CRITICAL APPLICATIONS
//  Xilinx products are not designed or intended to be fail-
//  safe, or for use in any application requiring fail-safe
//  performance, such as life-support or safety devices or
//  systems, Class III medical devices, nuclear facilities,
//  applications related to the deployment of airbags, or any
//  other applications that could lead to death, personal
//  injury, or severe property or environmental damage
//  (individually and collectively, "Critical
//  Applications"). Customer assumes the sole risk and
//  liability of any use of Xilinx products in Critical
//  Applications, subject only to applicable laws and
//  regulations governing limitations on product liability.
//
//  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
//  PART OF THIS FILE AT ALL TIMES. 
//-----------------------------------------------------------------------------
//
// Register Slice
//   Generic single-channel AXI pipeline register on forward and/or reverse signal path
//
// Verilog-standard:  Verilog 2001
//--------------------------------------------------------------------------
//
// Structure:
//   axic_register_slice
//
//--------------------------------------------------------------------------

`timescale 1ps/1ps
`default_nettype none

(* DowngradeIPIdentifiedWarnings="yes" *)
module axis_clock_converter_v1_1_14_axisc_sample_cycle_ratio # (
///////////////////////////////////////////////////////////////////////////////
// Parameter Definitions
///////////////////////////////////////////////////////////////////////////////
  parameter C_RATIO = 2 // Must be > 0
  )
 (
///////////////////////////////////////////////////////////////////////////////
// Port Declarations
///////////////////////////////////////////////////////////////////////////////
  input  wire                    SLOW_ACLK,
  input  wire                    FAST_ACLK,

  output wire                    SAMPLE_CYCLE_EARLY,
  output wire                    SAMPLE_CYCLE
);

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Local parameters
////////////////////////////////////////////////////////////////////////////////
localparam P_DELAY = C_RATIO > 2 ? C_RATIO-1 : C_RATIO-1; 
////////////////////////////////////////////////////////////////////////////////
// Wires/Reg declarations
////////////////////////////////////////////////////////////////////////////////
reg                slow_aclk_div2 = 0;
reg                posedge_finder_first;
reg                posedge_finder_second;
wire               first_edge;
wire               second_edge;
reg  [P_DELAY-1:0] sample_cycle_d;
(* shreg_extract = "no" *) reg                sample_cycle_r;


////////////////////////////////////////////////////////////////////////////////
// BEGIN RTL
////////////////////////////////////////////////////////////////////////////////
generate
  if (C_RATIO == 1) begin : gen_always_sample
    assign SAMPLE_CYCLE_EARLY = 1'b1;
    assign SAMPLE_CYCLE = 1'b1;
  end
  else begin : gen_sample_cycle
    genvar i;
    always @(posedge SLOW_ACLK) begin 
      slow_aclk_div2 <= ~slow_aclk_div2;
    end

    // Find matching rising edges by clocking slow_aclk_div2 onto faster clock
    always @(posedge FAST_ACLK) begin 
      posedge_finder_first <= slow_aclk_div2;
    end
    always @(posedge FAST_ACLK) begin 
      posedge_finder_second <= ~slow_aclk_div2;
    end

    assign first_edge = slow_aclk_div2 & ~posedge_finder_first;
    assign second_edge = ~slow_aclk_div2 & ~posedge_finder_second;

    always @(*) begin 
      sample_cycle_d[P_DELAY-1] = first_edge | second_edge;
    end
   
    // delay the posedge alignment by C_RATIO - 1 to set the sample cycle as
    // the clock one cycle before the posedge.
    for (i = P_DELAY-1; i > 0; i = i - 1) begin : gen_delay
      always @(posedge FAST_ACLK) begin
        sample_cycle_d[i-1] <= sample_cycle_d[i];
      end
    end

    always @(posedge FAST_ACLK) begin 
      sample_cycle_r <= sample_cycle_d[0];
    end
    assign SAMPLE_CYCLE_EARLY = sample_cycle_d[0];
    assign SAMPLE_CYCLE = sample_cycle_r;
  end
endgenerate

endmodule // axisc_sample_cycle_ratio

`default_nettype wire


//  (c) Copyright 2011-2013 Xilinx, Inc. All rights reserved.
//
//  This file contains confidential and proprietary information
//  of Xilinx, Inc. and is protected under U.S. and
//  international copyright and other intellectual property
//  laws.
//
//  DISCLAIMER
//  This disclaimer is not a license and does not grant any
//  rights to the materials distributed herewith. Except as
//  otherwise provided in a valid license issued to you by
//  Xilinx, and to the maximum extent permitted by applicable
//  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
//  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
//  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
//  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
//  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
//  (2) Xilinx shall not be liable (whether in contract or tort,
//  including negligence, or under any other theory of
//  liability) for any loss or damage of any kind or nature
//  related to, arising under or in connection with these
//  materials, including for any direct, or any indirect,
//  special, incidental, or consequential loss or damage
//  (including loss of data, profits, goodwill, or any type of
//  loss or damage suffered as a result of any action brought
//  by a third party) even if such damage or loss was
//  reasonably foreseeable or Xilinx had been advised of the
//  possibility of the same.
//
//  CRITICAL APPLICATIONS
//  Xilinx products are not designed or intended to be fail-
//  safe, or for use in any application requiring fail-safe
//  performance, such as life-support or safety devices or
//  systems, Class III medical devices, nuclear facilities,
//  applications related to the deployment of airbags, or any
//  other applications that could lead to death, personal
//  injury, or severe property or environmental damage
//  (individually and collectively, "Critical
//  Applications"). Customer assumes the sole risk and
//  liability of any use of Xilinx products in Critical
//  Applications, subject only to applicable laws and
//  regulations governing limitations on product liability.
//
//  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
//  PART OF THIS FILE AT ALL TIMES. 
//-----------------------------------------------------------------------------
//
// axis_clock_converter
//
// Verilog-standard:  Verilog 2001
//--------------------------------------------------------------------------
//
// Structure:
//   axis_clock_converter
//     util_aclken_converter (optional)
//     fifo_generator
//       or
//     axisc_sync_clock_converter
//     util_aclken_converter (optional)
//--------------------------------------------------------------------------

`timescale 1ps/1ps
`default_nettype none

(* DowngradeIPIdentifiedWarnings="yes" *)
module axis_clock_converter_v1_1_14_axis_clock_converter #
(
///////////////////////////////////////////////////////////////////////////////
// Parameter Definitions
///////////////////////////////////////////////////////////////////////////////
  parameter         C_FAMILY           = "virtex6",
  parameter integer C_AXIS_TDATA_WIDTH = 32,
  parameter integer C_AXIS_TID_WIDTH   = 1,
  parameter integer C_AXIS_TDEST_WIDTH = 1,
  parameter integer C_AXIS_TUSER_WIDTH = 1,
  parameter [31:0]  C_AXIS_SIGNAL_SET  = 32'hFF,
  // C_AXIS_SIGNAL_SET: each bit if enabled specifies which axis optional signals are present
  //   [0] => TREADY present
  //   [1] => TDATA present
  //   [2] => TSTRB present, TDATA must be present
  //   [3] => TKEEP present, TDATA must be present
  //   [4] => TLAST present
  //   [5] => TID present
  //   [6] => TDEST present
  //   [7] => TUSER present
  parameter integer C_IS_ACLK_ASYNC     = 1,
  parameter integer C_SYNCHRONIZER_STAGE = 2,
  parameter integer C_S_AXIS_ACLK_RATIO = 1,
  parameter integer C_M_AXIS_ACLK_RATIO = 2,
  parameter integer C_ACLKEN_CONV_MODE  = 0
  // C_ACLKEN_CONV_MODE: Determines how to handle the clock enable pins during
  // clock conversion
  // 0 -- Clock enables not converted
  // 1 -- s_axis_aclken can toggle,  m_axis_aclken always high.
  // 2 -- s_axis_aclken always high, m_axis_aclken can toggle.
  // 3 -- s_axis_aclken can toggle,  m_axis_aclken can toggle.
  )
  (
///////////////////////////////////////////////////////////////////////////////
// Port Declarations
///////////////////////////////////////////////////////////////////////////////
   // System Signals
   input wire                             s_axis_aresetn,
   input wire                             m_axis_aresetn,
   input wire                             s_axis_aclken,
   input wire                             m_axis_aclken,

   // Slave side
   input  wire                            s_axis_aclk,
   input  wire                            s_axis_tvalid,
   output wire                            s_axis_tready,
   input  wire [C_AXIS_TDATA_WIDTH-1:0]   s_axis_tdata,
   input  wire [C_AXIS_TDATA_WIDTH/8-1:0] s_axis_tstrb,
   input  wire [C_AXIS_TDATA_WIDTH/8-1:0] s_axis_tkeep,
   input  wire                            s_axis_tlast,
   input  wire [C_AXIS_TID_WIDTH-1:0]     s_axis_tid,
   input  wire [C_AXIS_TDEST_WIDTH-1:0]   s_axis_tdest,
   input  wire [C_AXIS_TUSER_WIDTH-1:0]   s_axis_tuser,

   // Master side
   input  wire                            m_axis_aclk,
   output wire                            m_axis_tvalid,
   input  wire                            m_axis_tready,
   output wire [C_AXIS_TDATA_WIDTH-1:0]   m_axis_tdata,
   output wire [C_AXIS_TDATA_WIDTH/8-1:0] m_axis_tstrb,
   output wire [C_AXIS_TDATA_WIDTH/8-1:0] m_axis_tkeep,
   output wire                            m_axis_tlast,
   output wire [C_AXIS_TID_WIDTH-1:0]     m_axis_tid,
   output wire [C_AXIS_TDEST_WIDTH-1:0]   m_axis_tdest,
   output wire [C_AXIS_TUSER_WIDTH-1:0]   m_axis_tuser

   );

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
`include "axis_infrastructure_v1_1_0.vh"

////////////////////////////////////////////////////////////////////////////////
// Local parameters
////////////////////////////////////////////////////////////////////////////////
localparam integer LP_S_ACLKEN_CAN_TOGGLE = (C_ACLKEN_CONV_MODE == 1) || (C_ACLKEN_CONV_MODE == 3);
localparam integer LP_M_ACLKEN_CAN_TOGGLE = (C_ACLKEN_CONV_MODE == 2) || (C_ACLKEN_CONV_MODE == 3);
localparam integer P_FIFO_MODE            = 1;  // clock converter use of fifo_gen will not 
                                                // use packet mode.                                           
localparam integer P_FIFO_DEPTH           = 32; // clock converter use of fifo_gen be fixed 
                                                // at 32 deep fifo.
                                          
localparam integer P_S_AXIS_ACLK_RATIO    = f_lcm(C_S_AXIS_ACLK_RATIO, C_M_AXIS_ACLK_RATIO) / C_M_AXIS_ACLK_RATIO;
localparam integer P_M_AXIS_ACLK_RATIO    = f_lcm(C_S_AXIS_ACLK_RATIO, C_M_AXIS_ACLK_RATIO) / C_S_AXIS_ACLK_RATIO;
                                          
localparam integer P_INST_FIFO_GEN        =  C_IS_ACLK_ASYNC || (P_S_AXIS_ACLK_RATIO != 1 && P_M_AXIS_ACLK_RATIO != 1);
                                          
localparam integer P_TPAYLOAD_WIDTH       = f_payload_width(C_AXIS_TDATA_WIDTH, C_AXIS_TID_WIDTH, 
                                                 C_AXIS_TDEST_WIDTH, C_AXIS_TUSER_WIDTH, 
                                                 C_AXIS_SIGNAL_SET);
localparam integer P_SAMPLE_CYCLE_RATIO   = (C_S_AXIS_ACLK_RATIO > C_M_AXIS_ACLK_RATIO) 
                                            ? C_S_AXIS_ACLK_RATIO : C_M_AXIS_ACLK_RATIO;


////////////////////////////////////////////////////////////////////////////////
// Wires/Reg declarations
////////////////////////////////////////////////////////////////////////////////
wire [P_TPAYLOAD_WIDTH-1:0] s_axis_tpayload;

wire [P_TPAYLOAD_WIDTH-1:0] d1_tpayload;
wire                        d1_tvalid;
wire                        d1_tready;

wire [P_TPAYLOAD_WIDTH-1:0] d2_tpayload;
wire                        d2_tvalid;
wire                        d2_tready;

wire [P_TPAYLOAD_WIDTH-1:0] m_axis_tpayload;


////////////////////////////////////////////////////////////////////////////////
// BEGIN RTL
////////////////////////////////////////////////////////////////////////////////



generate
  if (P_INST_FIFO_GEN) begin : gen_fifo_gen_ck_conv
    // Asynchronous and default is to use fifo_gen for clock conversion
    axis_data_fifo_v1_1_14_axis_data_fifo #(
      .C_FAMILY             ( C_FAMILY             ) ,
      .C_AXIS_TDATA_WIDTH   ( C_AXIS_TDATA_WIDTH   ) ,
      .C_AXIS_TID_WIDTH     ( C_AXIS_TID_WIDTH     ) ,
      .C_AXIS_TDEST_WIDTH   ( C_AXIS_TDEST_WIDTH   ) ,
      .C_AXIS_TUSER_WIDTH   ( C_AXIS_TUSER_WIDTH   ) ,
      .C_AXIS_SIGNAL_SET    ( C_AXIS_SIGNAL_SET    ) ,
      .C_FIFO_DEPTH         ( P_FIFO_DEPTH         ) ,
      .C_FIFO_MODE          ( P_FIFO_MODE          ) ,
      .C_IS_ACLK_ASYNC      ( 1                    ) , // enables clock cross when 1
      .C_SYNCHRONIZER_STAGE ( C_SYNCHRONIZER_STAGE ) ,
      .C_ACLKEN_CONV_MODE   ( C_ACLKEN_CONV_MODE   ) 
    )
    axis_data_fifo_0
    (
      .s_axis_aresetn ( s_axis_aresetn ) ,
      .m_axis_aresetn ( m_axis_aresetn ) ,
      .s_axis_aclk    ( s_axis_aclk    ) ,
      .s_axis_aclken  ( s_axis_aclken  ) ,
      .s_axis_tvalid  ( s_axis_tvalid  ) ,
      .s_axis_tready  ( s_axis_tready  ) ,
      .s_axis_tdata   ( s_axis_tdata   ) ,
      .s_axis_tstrb   ( s_axis_tstrb   ) ,
      .s_axis_tkeep   ( s_axis_tkeep   ) ,
      .s_axis_tlast   ( s_axis_tlast   ) ,
      .s_axis_tid     ( s_axis_tid     ) ,
      .s_axis_tdest   ( s_axis_tdest   ) ,
      .s_axis_tuser   ( s_axis_tuser   ) ,
      .m_axis_aclk    ( m_axis_aclk    ) ,
      .m_axis_aclken  ( m_axis_aclken  ) ,
      .m_axis_tvalid  ( m_axis_tvalid  ) ,
      .m_axis_tready  ( m_axis_tready  ) ,
      .m_axis_tdata   ( m_axis_tdata   ) ,
      .m_axis_tstrb   ( m_axis_tstrb   ) ,
      .m_axis_tkeep   ( m_axis_tkeep   ) ,
      .m_axis_tlast   ( m_axis_tlast   ) ,
      .m_axis_tid     ( m_axis_tid     ) ,
      .m_axis_tdest   ( m_axis_tdest   ) ,
      .m_axis_tuser   ( m_axis_tuser   ) ,
      .axis_data_count    (            ) ,
      .axis_wr_data_count (            ) ,
      .axis_rd_data_count (            ) 
    );
  end
  else begin : gen_sync_ck_conv

    wire                        slow_aclk;
    wire                        fast_aclk;
    wire                        sample_cycle_early;
    wire                        sample_cycle;

    axis_infrastructure_v1_1_0_util_axis2vector #(
      .C_TDATA_WIDTH    ( C_AXIS_TDATA_WIDTH ) ,
      .C_TID_WIDTH      ( C_AXIS_TID_WIDTH   ) ,
      .C_TDEST_WIDTH    ( C_AXIS_TDEST_WIDTH ) ,
      .C_TUSER_WIDTH    ( C_AXIS_TUSER_WIDTH ) ,
      .C_TPAYLOAD_WIDTH ( P_TPAYLOAD_WIDTH   ) ,
      .C_SIGNAL_SET     ( C_AXIS_SIGNAL_SET  ) 
    )
    util_axis2vector_0 (
      .TDATA    ( s_axis_tdata    ) ,
      .TSTRB    ( s_axis_tstrb    ) ,
      .TKEEP    ( s_axis_tkeep    ) ,
      .TLAST    ( s_axis_tlast    ) ,
      .TID      ( s_axis_tid      ) ,
      .TDEST    ( s_axis_tdest    ) ,
      .TUSER    ( s_axis_tuser    ) ,
      .TPAYLOAD ( s_axis_tpayload )
    );

    if (LP_S_ACLKEN_CAN_TOGGLE) begin : gen_s_aclken_converter
      axis_infrastructure_v1_1_0_util_aclken_converter #( 
        .C_PAYLOAD_WIDTH       ( P_TPAYLOAD_WIDTH       ) ,
        .C_S_ACLKEN_CAN_TOGGLE ( LP_S_ACLKEN_CAN_TOGGLE ) ,
        .C_M_ACLKEN_CAN_TOGGLE ( 0                      ) 
      )
      s_util_aclken_converter_0 ( 
        .ACLK      ( s_axis_aclk     ) ,
        .ARESETN   ( s_axis_aresetn  ) ,
        .S_ACLKEN  ( s_axis_aclken   ) ,
        .S_PAYLOAD ( s_axis_tpayload ) ,
        .S_VALID   ( s_axis_tvalid   ) ,
        .S_READY   ( s_axis_tready   ) ,
        .M_ACLKEN  ( 1'b1            ) ,
        .M_PAYLOAD ( d1_tpayload     ) ,
        .M_VALID   ( d1_tvalid       ) ,
        .M_READY   ( d1_tready       )
      );
    end
    else begin : gen_s_aclken_passthru
      assign d1_tpayload = s_axis_tpayload;
      assign d1_tvalid   = s_axis_tvalid;
      assign s_axis_tready = d1_tready;
    end

    // Sample cycle used to determine when to assert a signal on a fast clock
    // to be flopped onto a slow clock.
    assign slow_aclk   = (C_S_AXIS_ACLK_RATIO > C_M_AXIS_ACLK_RATIO) ? m_axis_aclk   : s_axis_aclk;
    assign fast_aclk   = (C_S_AXIS_ACLK_RATIO < C_M_AXIS_ACLK_RATIO) ? m_axis_aclk   : s_axis_aclk;

    axis_clock_converter_v1_1_14_axisc_sample_cycle_ratio #(
      .C_RATIO ( P_SAMPLE_CYCLE_RATIO )
    ) 
    axisc_sample_cycle_ratio_m (
      .SLOW_ACLK          ( slow_aclk          ) ,
      .FAST_ACLK          ( fast_aclk          ) ,
      .SAMPLE_CYCLE_EARLY ( sample_cycle_early ) ,
      .SAMPLE_CYCLE       ( sample_cycle       ) 
    );

    axis_clock_converter_v1_1_14_axisc_sync_clock_converter #( 
      .C_FAMILY        ( C_FAMILY            ) ,
      .C_PAYLOAD_WIDTH ( P_TPAYLOAD_WIDTH    ) ,
      .C_S_ACLK_RATIO  ( P_S_AXIS_ACLK_RATIO ) ,
      .C_M_ACLK_RATIO  ( P_M_AXIS_ACLK_RATIO ) ,
      .C_MODE          ( 1 )
    )
    axisc_sync_clock_converter_0 (
      .SAMPLE_CYCLE_EARLY ( sample_cycle_early ) ,
      .SAMPLE_CYCLE       ( sample_cycle       ) ,
      .S_ACLK             ( s_axis_aclk        ) ,
      .S_ARESETN          ( s_axis_aresetn     ) ,
      .S_PAYLOAD          ( d1_tpayload        ) ,
      .S_VALID            ( d1_tvalid          ) ,
      .S_READY            ( d1_tready          ) ,
      .M_ACLK             ( m_axis_aclk        ) ,
      .M_ARESETN          ( m_axis_aresetn     ) ,
      .M_PAYLOAD          ( d2_tpayload        ) ,
      .M_VALID            ( d2_tvalid          ) ,
      .M_READY            ( d2_tready          ) 
    );

    if (LP_M_ACLKEN_CAN_TOGGLE) begin : gen_m_aclken_converter
      axis_infrastructure_v1_1_0_util_aclken_converter #( 
        .C_PAYLOAD_WIDTH       ( P_TPAYLOAD_WIDTH       ) ,
        .C_S_ACLKEN_CAN_TOGGLE ( 0                      ) ,
        .C_M_ACLKEN_CAN_TOGGLE ( LP_M_ACLKEN_CAN_TOGGLE ) 
      )
      m_util_aclken_converter_0 ( 
        .ACLK      ( m_axis_aclk     ) ,
        .ARESETN   ( m_axis_aresetn  ) ,
        .S_ACLKEN  ( 1'b1            ) ,
        .S_PAYLOAD ( d2_tpayload     ) ,
        .S_VALID   ( d2_tvalid       ) ,
        .S_READY   ( d2_tready       ) ,
        .M_ACLKEN  ( m_axis_aclken   ) ,
        .M_PAYLOAD ( m_axis_tpayload ) ,
        .M_VALID   ( m_axis_tvalid   ) ,
        .M_READY   ( m_axis_tready   ) 
      );
    end
    else begin : gen_m_aclken_passthru
      assign m_axis_tpayload = d2_tpayload;
      assign m_axis_tvalid = d2_tvalid;
      assign d2_tready = m_axis_tready;
    end

    axis_infrastructure_v1_1_0_util_vector2axis #(
      .C_TDATA_WIDTH    ( C_AXIS_TDATA_WIDTH ) ,
      .C_TID_WIDTH      ( C_AXIS_TID_WIDTH   ) ,
      .C_TDEST_WIDTH    ( C_AXIS_TDEST_WIDTH ) ,
      .C_TUSER_WIDTH    ( C_AXIS_TUSER_WIDTH ) ,
      .C_TPAYLOAD_WIDTH ( P_TPAYLOAD_WIDTH   ) ,
      .C_SIGNAL_SET     ( C_AXIS_SIGNAL_SET  ) 
    )
    util_vector2axis_0 (
      .TPAYLOAD ( m_axis_tpayload ) ,
      .TDATA    ( m_axis_tdata    ) ,
      .TSTRB    ( m_axis_tstrb    ) ,
      .TKEEP    ( m_axis_tkeep    ) ,
      .TLAST    ( m_axis_tlast    ) ,
      .TID      ( m_axis_tid      ) ,
      .TDEST    ( m_axis_tdest    ) ,
      .TUSER    ( m_axis_tuser    ) 
    );
  end

endgenerate



endmodule // axis_clock_converter

`default_nettype wire


