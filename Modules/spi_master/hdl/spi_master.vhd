----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 21.12.2023 17:22:47
-- Design Name: 
-- Module Name: spi_master - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity spi_master is
  Port ( 
         -- Basic
         clk   : in std_logic;
         reset : in std_logic;
         
         -- Control
         start : in  std_logic;
         ready : out std_logic;
         
         -- SPI
         mosi         : out std_logic;
         sck          : out std_logic;
         cs           : out std_logic;
         data_command : out std_logic;
         
         -- Data
         data : in std_logic_vector(7 downto 0)
        );
end spi_master;

architecture Behavioral of spi_master is
    
    -- Signals
    signal done_signal: std_logic;
    
    
    -- FSM
    type state_t is (s_ready, s_busy);
    signal state : state_t := s_ready;
    
begin
    
    
    FSM_proc: process(clk, reset)
    begin
        if (reset = '1') then
            state <= s_ready;
            ready <= '0';
        elsif (rising_edge(clk)) then
            -- State transition
            case state is
                when s_ready =>
                    if (start = '1') then
                        state <= s_busy;
                        ready <= '0';
                    end if;
                when s_busy =>
                    if (done_signal = '1') then
                        state <= s_ready;
                        ready <= '1';
                    end if;
            end case;
        end if;
    end process;

    -- sck generation
    sck <= clk when (state = s_busy) else '1';
            
end Behavioral;
