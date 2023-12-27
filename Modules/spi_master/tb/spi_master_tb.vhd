----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 21.12.2023 18:09:46
-- Design Name: 
-- Module Name: spi_master_tb - Behavioral
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

entity spi_master_tb is
--  Port ( );
end spi_master_tb;

architecture Behavioral of spi_master_tb is

-- Instantiate the COmponent Under Test
component spi_master is
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
         
         -- Data
         data : in std_logic_vector(7 downto 0);
         
         -- Debug
         bit_counter : out std_logic_vector(3 downto 0);
         shift_data  : out std_logic_vector(7 downto 0)
        );
end component;
    
    -- Clock
    constant clk_period : time := 1 ns;
    
    -- Signals
    signal clk, reset, start, ready, mosi, sck, cs : std_logic;
    signal data : std_logic_vector(7 downto 0);
    
    signal bit_counter : std_logic_vector(3 downto 0);
    signal shift_data : std_logic_vector(7 downto 0);
    
begin
    
    -- Port Mapping
    UUT: spi_master
        Port Map( clk   => clk,
                  reset => reset,
                  
                  -- Control
                  start => start,
                  ready => ready,
                  
                  -- SPI
                  mosi         => mosi,
                  sck          => sck,
                  cs           => cs,
                  
                  -- Data
                  data => data,
                  
                  -- Debug
                  bit_counter => bit_counter,
                  shift_data  => shift_data
                );
                
    -- Stimulus processes
                  
    clk_proc: process
    begin
        clk <= '0';
            wait for clk_period/2;
        clk <= '1';
            wait for clk_period/2;
    end process;
    
    stim_proc: process
    begin
    reset     <= '1';
    start     <= '0';
    data      <= "00000000";
    wait for 2*clk_period; 
    reset     <= '0';
    start     <= '0';
    data      <= "10011001";
    wait for 2*clk_period;
    reset     <= '0';
    start     <= '1';
    wait for 1*clk_period;
    reset     <= '0';
    start     <= '0';
    wait for 20*clk_period;
    reset     <= '0';
    start     <= '1';
    data      <= "00110010";
    wait for 1*clk_period;
    reset     <= '0';
    start     <= '0';
    wait;
    
    
    end process;
    
end Behavioral;
