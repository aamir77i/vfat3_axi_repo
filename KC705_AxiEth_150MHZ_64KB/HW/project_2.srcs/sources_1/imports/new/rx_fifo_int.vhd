----------------------------------------------------------------------------------
-- Company: IIHE - ULB and CERN
-- Engineers: M. Dabrowski and J. Rosa 
-- 
-- Create Date: 25.04.2017 09:46:28
-- Design Name: 
-- Module Name: rx_fifo_int
-- Project Name: VFAT3 TESTING FIRMWARE
-- Target Devices: Kintex-7 KC705 Evaluation Platform
-- Tool Versions: Vivado 2016.3
-- Description: Block transmitting data from deserializer (VFAT3) to the RX_fifo. Also filters the fillers
-- 
-- Dependencies: 
-- 
-- Revision: FINAL
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------

library IEEE;
library work;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use ieee.numeric_std.all;
--use work.cst_pkg.all;

entity rx_fifo_int is
	generic(
		cmd_w	: natural :=  8;--CMD_VFAT_W;
		fifo_w	: natural := 32;--FIFO_W;
		BCd_w	: natural := 24--BCD_O_W
	);
	port(
		clk 	: in std_logic;
		rst_n 	: in std_logic;
		d_in	: in std_logic_vector(cmd_w - 1 downto 0);
		bit_aligned   : in std_logic;
		d_out	: out std_logic_vector(fifo_w - 1 downto 0);
		dv		: out std_logic
	);
end rx_fifo_int;

architecture rtl of rx_fifo_int is
	
	type state_type is (IDLE,FILTER_DATA, SEND_PACKET);
	signal d_in_n, d_in_p : std_logic_vector(cmd_w - 1 downto 0);
	signal state_n, state_p : state_type;
	signal BCd_n, BCd_p : std_logic_vector(BCd_w - 1 downto 0);
	signal cnt_n, cnt_p : std_logic_vector(5 downto 0);
	constant F1 : std_logic_vector(7 downto 0) := X"7e";
	constant F2 : std_logic_vector(7 downto 0) := X"81";
--	constant FCC_A : std_logic_vector(7 downto 0) := X"00";-- to remove, only for loopback
--	constant FCC_P : std_logic_vector(7 downto 0) := X"ff"; -- to remove, only for loopback
	constant H0A : std_logic_vector(7 downto 0) := X"1e";
	constant H0B : std_logic_vector(7 downto 0) := X"5e";
	constant H1A : std_logic_vector(7 downto 0) := X"1a";
	constant H1B : std_logic_vector(7 downto 0) := X"56";
	constant NUM_BYTES : std_logic_vector(4 downto 0) := "11001";
	
	begin
	
		d_in_n <= d_in;
		d_out <= BCd_p & d_in_p;
		BCd_n <= BCd_p+1;
	
	combinatorial:process
		
	begin
		state_n <= state_p;
		cnt_n <= cnt_p;
		dv <= '0';
		
		case(state_p) is 
		
		    when IDLE =>
		       if bit_aligned = '1' then
		          state_n <= FILTER_DATA;
		      else 
		          state_n <= IDLE;
		      end if;
			when FILTER_DATA =>
				if((d_in_p /= F1) and (d_in_p /= F2)) then --  and (d_in_p /= FCC_A) and (d_in_p /= FCC_P)
					dv <= '1';
					if ( (d_in_p = H0A) or (d_in_p = H0B) or (d_in_p = H1A)  or (d_in_p = H1B) ) then
						state_n <= SEND_PACKET;
						cnt_n <= (others => '0');
					end if;
				end if;
			when SEND_PACKET =>
				dv <= '1';
				if(cnt_p >= NUM_BYTES) then
					state_n <= FILTER_DATA;
				else
					cnt_n <= cnt_p+1;
				end if;
		end case;
	end process;
	
	sequential: process(clk,rst_n)
	
	begin
	
		if rising_edge(clk) then
			if rst_n = '0' then
				d_in_p <= (others => '0');
				state_p <= IDLE;
				BCd_p <= BCd_n;
				cnt_p <= (others => '0');
			else
				d_in_p <= d_in_n;
				state_p <= state_n;
				BCd_p <= BCd_n;
				cnt_p <= cnt_n;
			end if;
			
		end if;
		
	end process;

end rtl;

